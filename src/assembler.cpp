#include "assembler.h"

uint16_t Assembler::EncodeInstructionWord(const OpCode &opCode, const std::array<RegisterId, 3> &args)
{
    uint16_t instructionWord = (opCode.opCode << (opCode.argCount * 4));
    for (auto i = 0; i < opCode.argCount; ++i)
    {
        // If we ever hit this, something went wrong.
        assert(opCode.argCount >= (i + 1));

        auto nArgShiftCount = opCode.argCount - (i + 1);
        instructionWord |= (static_cast<uint16_t>(args[i]) << (nArgShiftCount * 4));
    }
    return instructionWord;
}

uint16_t Assembler::GetValueFromStringLiteral(std::string literal) const
{
    const std::string hexLiteral = "h'";
    uint16_t unsignedValue = 0;
    int16_t signedValue = 0;

    // First check if it's a decimal or hex
    auto hexIndex = literal.find(hexLiteral);
    if (hexIndex != std::string::npos)
    {
        // Hex doesn't accept negatives
        literal.erase(hexIndex, hexLiteral.length());
        if (!std::regex_match(literal, std::regex("[0-9a-fA-F]+")))
        {
            throw std::invalid_argument("Invalid hex number " + literal);
        }

        std::stringstream ssLiteral(literal);
        ssLiteral >> std::hex >> unsignedValue;
        return unsignedValue;
    }
    if (!std::regex_match(literal, std::regex("[-]?[0-9]+")))
    {
        throw std::invalid_argument("Invalid number " + literal);
    }

    std::stringstream ssLiteral(literal);
    if (literal.find("-") != std::string::npos)
    {
        // Keep the bit ordering
        ssLiteral >> signedValue;
        std::memcpy(&unsignedValue, &signedValue, sizeof(unsignedValue));
    }
    else
    {
        ssLiteral >> unsignedValue;
    }
    return unsignedValue;
}

uint16_t Assembler::_WordToBigEndian(uint16_t word) const
{
    uint16_t result = word << 8;
    word >>= 8;
    result |= word;
    return result;
}

uint16_t Assembler::EncodeInstructionWord(std::string instruction, uint16_t instructionIndex)
{
    instruction = _RemoveComments(instruction);
    // We have at max 3 arguments on any instruction
    std::array<std::string, 3> stringArgs;
    std::array<RegisterId, 3> registerArgs;

    // Let's remove any commas first
    std::replace(instruction.begin(), instruction.end(), ',', ' ');

    // Let's check for goto tags
    if (instruction.find("goto:") != std::string::npos)
    {
        instruction = std::regex_replace(instruction, std::regex("goto:"), "SET ");
        // + 4 because the set will take 2 words (4 bytes). And we want to goto the instruction next, not the SET
        // +2 would jump to the literal of the SET instr (the 2nd word)
        instruction = instruction + " " + std::to_string(instructionIndex + 4);
    }

    std::stringstream line(instruction);
    std::string mnemonic;
    line >> mnemonic;

    if (opCodeTable.count(mnemonic) < 1)
    {
        throw std::runtime_error("Error: unrecognized mnemonic " + mnemonic);
    }

    const OpCode &opCode = opCodeTable[mnemonic];

    if (_IsSpecialInstruction(opCode.id))
    {
        line >> stringArgs[0];
        line >> stringArgs[1];

        const auto &regName = stringArgs[0];
        const auto &stringLiteral = stringArgs[1];

        if (registerMap.count(regName) < 1)
        {
            throw std::runtime_error("Error: unrecognized register named " + regName);
        }

        registerArgs[0] = reinterpret_cast<RegisterId>(registerMap[regName]);

        _literalValue = GetValueFromStringLiteral(stringLiteral);
        _pendingLiteralValue = true;

        // Check if it has anything other than spaces or \n
        if (line >> stringArgs[0])
        {
            throw std::runtime_error("Instruction has more operators than expected.");
        }
        return EncodeInstructionWord(opCode, registerArgs);
    }

    for (unsigned i = 0; i < opCode.argCount; ++i)
    {
        line >> stringArgs[i];
        if (registerMap.count(stringArgs[i]) < 1)
        {
            throw std::runtime_error("Error: unrecognized register named " + stringArgs[i]);
        }

        registerArgs[i] = reinterpret_cast<RegisterId>(registerMap[stringArgs[i]]);
    }

    // Check if it has anything other than spaces or \n
    if (line >> stringArgs[0])
    {
        throw std::runtime_error("Instruction has more operators than expected.");
    }
    return EncodeInstructionWord(opCode, registerArgs);
}

std::vector<uint16_t> Assembler::AssembleFile()
{
    std::string stringLine;
    std::string stringProgram;

    // This will keep track of the address the instruction is to be written
    uint16_t instructionAddress = 0;
    if (!_canWriteFiles)
    {
        throw std::logic_error("No files were given to read and write.");
    }

    _inFileStream.open(_inFilename);
    if (!_inFileStream)
    {
        throw std::runtime_error("File does not exist, or cannot be opened.");
    }
    while (std::getline(_inFileStream, stringLine))
    {
        stringProgram += "\n" + stringLine;
    }

    _inFileStream.close();
    return AssembleString(stringProgram);
}

std::vector<uint16_t> Assembler::AssembleString(std::string program)
{
    std::stringstream ssProgram(program);
    std::string stringLine;
    std::vector<uint16_t> binProgram;

    // Address for the instruction to be written.
    uint16_t instructionIndex = 0;

    // 1 because we're counting lines to report to the user
    for (unsigned i = 1; std::getline(ssProgram, stringLine); ++i)
    {
        if (!_ContainsInstruction(stringLine))
        {
            continue;
        }

        try
        {
            auto instr = EncodeInstructionWord(stringLine, instructionIndex);
            binProgram.push_back(_WordToBigEndian(instr));
            instructionIndex += 2;
            if (_pendingLiteralValue)
            {
                binProgram.push_back(_WordToBigEndian(_literalValue));
                _pendingLiteralValue = false;
                instructionIndex += 2;
            }
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error(e.what() + std::string(" on line i"));
        }
    }
    return binProgram;
}
bool Assembler::_ContainsInstruction(std::string line) const
{
    line = std::regex_replace(line, std::regex(";.*"), "");
    if (!std::regex_search(line.begin(), line.end(), std::regex("[a-zA-Z]+")))
    {
        return false;
    }
    return true;
}

void Assembler::WriteBinaryFile(std::vector<uint16_t> &program)
{
    _outFileStream.open(_outFilename, std::ios::trunc | std::ios::binary);
    if (!_outFileStream)
    {
        throw std::runtime_error("Cannot create or write to output file.");
    }

    _outFileStream.write(reinterpret_cast<char *>(&(program[0])), program.size() * sizeof(uint16_t));

    _inFileStream.close();
    _outFileStream.close();
}

bool Assembler::_IsSpecialInstruction(OpCodeId id) const
{
    // Remember that SET, and SET_M are special commands. We'll process them separately
    return (id == OpCodeId::SET) || (id == OpCodeId::SET_M);
}

std::string Assembler::_RemoveComments(std::string str) const
{
    return str.substr(0, str.find(";"));
}