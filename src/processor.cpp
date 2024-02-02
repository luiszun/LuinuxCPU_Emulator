#include "processor.h"
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
    // Initialize the registers
    static_assert(static_cast<uint8_t>(RegisterId::RAC) == 0);
    for (auto i = RegisterId::RAC; i != RegisterId::END_OF_REGLIST; i = RegisterId(static_cast<uint8_t>(i) + 1))
    {
        _registers.insert({i, Register(static_cast<uint8_t>(i) * uint8_t{2}, _internalMemory, i)});
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

uint16_t Processor::_DereferenceRegisterRead(RegisterId reg)
{
    const auto address = _registers.at(reg).Read();
    return _mainMemory.Read16(address);
}

void Processor::_DereferenceRegisterWrite(RegisterId reg, uint16_t value)
{
    const auto address = _registers.at(reg).Read();
    _mainMemory.Write16(address, value);
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
    // Let's create a table of function pointers to the instructions. That way we only reference those by OpCodeId
    typedef void (Processor::*OpFunction)(std::vector<std::shared_ptr<Register>>);
    static const std::unordered_map<OpCodeId, OpFunction> opCodeFunctionTable = {{OpCodeId::ADD, &Processor::ADD},
                                                                                 {OpCodeId::SUB, &Processor::SUB},
                                                                                 {OpCodeId::MUL, &Processor::MUL},
                                                                                 {OpCodeId::SET, &Processor::SET}};

    _instructionStatus = InstructionCycle::Execute;
    assert(_decodedOpCodeId != OpCodeId::INVALID_INSTR);
    std::vector<std::shared_ptr<Register>> argumentsAsRegisters;

    // Pack the arguments as pointers to the actual registers
    for (auto i = 0; i < _instructionArgs.size(); ++i)
    {
        argumentsAsRegisters.push_back(std::make_shared<Register>(_registers.at(_instructionArgs.at(i))));
    }

    assert(opCodeFunctionTable.count(_decodedOpCodeId) > 0);

    _CleanInstructionCycle();
}

void Processor::ADD(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(opA->Read() + opB->Read());
}
void Processor::SUB(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(opA->Read() - opB->Read());
}
void Processor::MUL(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(opA->Read() * opB->Read());
}

void Processor::STOP(std::vector<std::shared_ptr<Register>> args)
{
    // args is not really used, but works for our table of ptrs to funcs.
    _instructionStatus = InstructionCycle::Halted;
}

void Processor::DIV(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(opA->Read() / opB->Read());
}
void Processor::AND(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(opA->Read() & opB->Read());
}
void Processor::OR(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(opA->Read() | opB->Read());
}
void Processor::XOR(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(opA->Read() ^ opB->Read());
}
void Processor::JZ(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    if (0 == opA)
    {
        auto opB = args.at(1);
        WriteRegister(RegisterId::RIP, opB->Read());
    }
}
void Processor::JNZ(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    if (0xffff == opA->Read())
    {
        auto opB = args.at(1);
        WriteRegister(RegisterId::RIP, opB->Read());
    }
}
void Processor::MOV(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
    opB->Write(opA->Read());
}
void Processor::LOAD(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);

    uint16_t valueAtAddress = _DereferenceRegisterRead(opA->registerId);
    opB->Write(valueAtAddress);
}
void Processor::STOR(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
}
void Processor::TSTB(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto opB = args.at(1);
}
void Processor::SETZ(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    opA->Write(0x0);
}
void Processor::SETO(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    opA->Write(0xffff);
}
void Processor::SET(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    opA->Write(_2wordOperand);
}
void Processor::PUSH(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto &RSP = _registers.at(RegisterId::RSP);
    _DereferenceRegisterWrite(RSP.registerId, opA->Read());

    RSP.Write(RSP.Read() + 2);
}
void Processor::POP(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    auto &RSP = _registers.at(RegisterId::RSP);
    RSP.Write(RSP.Read() - 2);

    opA->Write(_DereferenceRegisterRead(RSP.registerId));
}
void Processor::NOT(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    opA->Write(~(opA->Read()));
}
void Processor::SHFR(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    opA->Write(opA->Read() >> 1);
}
void Processor::SHFL(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    opA->Write(opA->Read() << 1);
}
void Processor::INC(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    opA->Write(opA->Read() + 1);
}
void Processor::DEC(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0);
    opA->Write(opA->Read() - 1);
}
void Processor::NOP(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::ADD_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::ADD_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::ADD_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::SUB_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::SUB_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::SUB_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::MUL_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::MUL_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::MUL_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::DIV_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::DIV_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::DIV_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::AND_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::AND_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::AND_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::OR_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::OR_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::OR_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::XOR_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::XOR_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::XOR_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::JZ_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::JZ_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::JZ_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::JNZ_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::JNZ_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::JNZ_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::MOV_RM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::MOV_MR(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::MOV_MM(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::TSTB_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::SETZ_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::SETO_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::SET_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::PUSH_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::POP_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::NOT_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::SHFR_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::SHFL_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::INC_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
void Processor::DEC_M(std::vector<std::shared_ptr<Register>> args)
{
    assert(true); // not implemented
}
