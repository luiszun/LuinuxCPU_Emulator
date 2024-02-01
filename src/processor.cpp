#include "processor.h"
#include "instruction_impl.h"
#include "common.h"

void Processor::WriteRegister(RegisterId reg, uint16_t value)
{
    _registers.at(reg).Write(value);
}

uint16_t Processor::ReadRegister(RegisterId reg) const
{
    return _registers.at(reg).Read();
}

void Processor::PerformExecutionCycle()
{
    _FetchInstruction();
    _DecodeInstruction();
    _ExecuteInstruction();

    // Instruction cycle is done at this point
    _instructionStatus = InstructionCycle::Idle;
}

Processor::Processor(Memory16 &programMemory)
    : _internalMemory(InternalMemorySize), _mainMemory(MainMemorySize), _programMemory(programMemory)
{
    static_assert(static_cast<uint8_t>(RegisterId::RAC) == 0);
    for (auto i = RegisterId::RAC; i != RegisterId::END_OF_REGLIST; i = RegisterId(static_cast<uint8_t>(i) + 1))
    {
        _registers.insert({i, Register(static_cast<uint8_t>(i) * uint8_t{2}, _internalMemory)});
    }

    WriteRegister(RegisterId::RSP, RSP_DefaultAddress);
    WriteRegister(RegisterId::RIP, 0);
}

uint16_t Processor::_ReadMemoryWord(Memory16 &memory, uint16_t address) const
{
    uint16_t value = (memory.Read8(address) << 8);
    value |= memory.Read8(address + 1);
    return value;
}

void Processor::_WriteMemoryWord(Memory16 &memory, uint16_t address, uint16_t value)
{
    memory.Write16(address, value);
}

void Processor::_CleanInstructionCycle()
{
    _decodedOpCodeId = OpCodeId::INVALID_INSTR;
    _instructionArgs.resize(0);
}

uint16_t Processor::_DereferenceRegister(RegisterId reg)
{
    const auto address = _registers.at(reg).Read();
    return (_mainMemory.Read8(address) << 8) | _mainMemory.Read8(address + 1);
}

void Processor::_FetchInstruction()
{
    _instructionStatus = InstructionCycle::Fetch;
    uint16_t ripVal = ReadRegister(RegisterId::RIP);
    _fetchedInstruction = _ReadMemoryWord(_programMemory, ripVal);
    WriteRegister(RegisterId::RIP, ripVal + sizeof(uint16_t));
}

void Processor::_DecodeInstruction()
{
    _instructionStatus = InstructionCycle::Decode;
    if ((_decodedOpCodeId != OpCodeId::INVALID_INSTR) || (_instructionArgs.size() > 0))
    {
        throw std::runtime_error("Decoding new instruction with previous exec cycle unfinished.");
    }
    // Need to check a map, opcode 4b, 8b, 12b, 16b
    for (auto i = 0; i < 4; ++i)
    {
        uint16_t opCodeValue = _fetchedInstruction >> (4 * i);
        if (opCodeValuesTable.count(opCodeValue) > 0)
        {
            _decodedOpCodeId = opCodeValuesTable.at(opCodeValue);
            break;
        }
    }

    // If we couldn't find the opcode, then we got an invalid operation. Throw for now. We still don't know how to
    // handle these.
    if (_decodedOpCodeId == OpCodeId::INVALID_INSTR)
    {
        throw std::runtime_error("Invalid instruction found in memory. Cannot decode");
    }
    _instructionArgs.resize(opCodeTable.at(_decodedOpCodeId).argCount);

    if ((_decodedOpCodeId == OpCodeId::SET) || _decodedOpCodeId == OpCodeId::SET_M)
    {
        _instructionArgs.resize(1);
        _instructionArgs[0] = static_cast<RegisterId>(_fetchedInstruction & 0xf);
        // We need to read the next word for these ones
        _FetchInstruction();

        _2wordOperand = _fetchedInstruction;
    }
    else
    {
        for (auto i = _instructionArgs.size() - 1; i >= 0; --i)
        {
            _instructionArgs[i] = static_cast<RegisterId>(_fetchedInstruction & 0xf);
            _fetchedInstruction >>= 4;
        }
        if (_fetchedInstruction != opCodeTable.at(_decodedOpCodeId).opCode)
        {
            throw std::runtime_error("Something went wrong. Did we decode more or less params than expected?");
        }
    }
}

void Processor::_ExecuteInstruction()
{
    _instructionStatus = InstructionCycle::Execute;

    // TODO: End of instruction cycle
    _CleanInstructionCycle();
}
