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
    if (_instructionStatus == InstructionCycle::Halted)
    {
        return;
    }
    _DoPerformExecutionCycle();
}

void Processor::_DoPerformExecutionCycle()
{
    _FetchInstruction();
    _DecodeInstruction();
    _ExecuteInstruction();

    // Instruction cycle is done at this point, break only on halted state
    if (_instructionStatus != InstructionCycle::Halted)
    {
        _instructionStatus = InstructionCycle::Idle;
    }
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

void Processor::_CleanInstructionCycle()
{
    _decodedOpCodeId = OpCodeId::INVALID_INSTR;
    _instructionArgs.resize(0);
}

uint16_t Processor::_DereferenceRegisterRead(RegisterId reg) const
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
    _fetchedInstruction = _programMemory.Read16(ripVal);
    WriteRegister(RegisterId::RIP, ripVal + sizeof(uint16_t));
}

void Processor::_DecodeInstruction()
{
    _instructionStatus = InstructionCycle::Decode;
    if ((_decodedOpCodeId != OpCodeId::INVALID_INSTR) || (_instructionArgs.size() > 0))
    {
        throw std::runtime_error("Decoding new instruction with previous exec cycle unfinished.");
    }

    // We check this case for leading zeroes, the rest of instructions can be determined below.`
    if ((_fetchedInstruction & 0xf000) == 0)
    {
        _decodedOpCodeId = OpCodeId::ADD;
    }
    else
    {
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
        // int i because we want to know when it reaches -1 as opposed to it running amok by rollover
        for (int i = _instructionArgs.size() - 1; i >= 0; --i)
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
    static const std::unordered_map<OpCodeId, OpFunction> opCodeFunctionTable = {
        {OpCodeId::ADD, &Processor::ADD},       {OpCodeId::SUB, &Processor::SUB},
        {OpCodeId::MUL, &Processor::MUL},       {OpCodeId::DIV, &Processor::DIV},
        {OpCodeId::AND, &Processor::AND},       {OpCodeId::OR, &Processor::OR},
        {OpCodeId::XOR, &Processor::XOR},       {OpCodeId::JZ, &Processor::JZ},
        {OpCodeId::JNZ, &Processor::JNZ},       {OpCodeId::MOV, &Processor::MOV},
        {OpCodeId::LOAD, &Processor::LOAD},     {OpCodeId::STOR, &Processor::STOR},
        {OpCodeId::TSTB, &Processor::TSTB},     {OpCodeId::SETZ, &Processor::SETZ},
        {OpCodeId::SETO, &Processor::SETO},     {OpCodeId::SET, &Processor::SET},
        {OpCodeId::PUSH, &Processor::PUSH},     {OpCodeId::POP, &Processor::POP},
        {OpCodeId::NOT, &Processor::NOT},       {OpCodeId::SHFR, &Processor::SHFR},
        {OpCodeId::SHFL, &Processor::SHFL},     {OpCodeId::INC, &Processor::INC},
        {OpCodeId::DEC, &Processor::DEC},       {OpCodeId::NOP, &Processor::NOP},
        {OpCodeId::STOP, &Processor::STOP},     {OpCodeId::ADD_RM, &Processor::ADD_RM},
        {OpCodeId::ADD_MR, &Processor::ADD_MR}, {OpCodeId::ADD_MM, &Processor::ADD_MM},
        {OpCodeId::SUB_RM, &Processor::SUB_RM}, {OpCodeId::SUB_MR, &Processor::SUB_MR},
        {OpCodeId::SUB_MM, &Processor::SUB_MM}, {OpCodeId::MUL_RM, &Processor::MUL_RM},
        {OpCodeId::MUL_MR, &Processor::MUL_MR}, {OpCodeId::MUL_MM, &Processor::MUL_MM},
        {OpCodeId::DIV_RM, &Processor::DIV_RM}, {OpCodeId::DIV_MR, &Processor::DIV_MR},
        {OpCodeId::DIV_MM, &Processor::DIV_MM}, {OpCodeId::AND_RM, &Processor::AND_RM},
        {OpCodeId::AND_MR, &Processor::AND_MR}, {OpCodeId::AND_MM, &Processor::AND_MM},
        {OpCodeId::OR_RM, &Processor::OR_RM},   {OpCodeId::OR_MR, &Processor::OR_MR},
        {OpCodeId::OR_MM, &Processor::OR_MM},   {OpCodeId::XOR_RM, &Processor::XOR_RM},
        {OpCodeId::XOR_MR, &Processor::XOR_MR}, {OpCodeId::XOR_MM, &Processor::XOR_MM},
        {OpCodeId::JZ_RM, &Processor::JZ_RM},   {OpCodeId::JZ_MR, &Processor::JZ_MR},
        {OpCodeId::JZ_MM, &Processor::JZ_MM},   {OpCodeId::JNZ_RM, &Processor::JNZ_RM},
        {OpCodeId::JNZ_MR, &Processor::JNZ_MR}, {OpCodeId::JNZ_MM, &Processor::JNZ_MM},
        {OpCodeId::MOV_RM, &Processor::MOV_RM}, {OpCodeId::MOV_MR, &Processor::MOV_MR},
        {OpCodeId::MOV_MM, &Processor::MOV_MM}, {OpCodeId::TSTB_M, &Processor::TSTB_M},
        {OpCodeId::SETZ_M, &Processor::SETZ_M}, {OpCodeId::SETO_M, &Processor::SETO_M},
        {OpCodeId::SET_M, &Processor::SET_M},   {OpCodeId::PUSH_M, &Processor::PUSH_M},
        {OpCodeId::POP_M, &Processor::POP_M},   {OpCodeId::NOT_M, &Processor::NOT_M},
        {OpCodeId::SHFR_M, &Processor::SHFR_M}, {OpCodeId::SHFL_M, &Processor::SHFL_M},
        {OpCodeId::INC_M, &Processor::INC_M},   {OpCodeId::DEC_M, &Processor::DEC_M}};
    _instructionStatus = InstructionCycle::Execute;
    assert(_decodedOpCodeId != OpCodeId::INVALID_INSTR);
    std::vector<std::shared_ptr<Register>> argumentsAsRegisters;

    // Pack the arguments as pointers to the actual registers
    for (auto i = 0; i < _instructionArgs.size(); ++i)
    {
        argumentsAsRegisters.push_back(std::make_shared<Register>(_registers.at(_instructionArgs.at(i))));
    }

    assert(opCodeFunctionTable.count(_decodedOpCodeId) > 0);
    OpFunction fPtr = opCodeFunctionTable.at(_decodedOpCodeId);

    (*this.*fPtr)(argumentsAsRegisters);

    _CleanInstructionCycle();
}

ConstantPair Processor::_Get_RR(std::vector<std::shared_ptr<Register>> args) const
{
    auto opA = args.at(0)->Read();
    auto opB = args.at(1)->Read();
    return std::make_pair(opA, opB);
}
ConstantPair Processor::_Get_MR(std::vector<std::shared_ptr<Register>> args) const
{
    auto opA = _DereferenceRegisterRead(args.at(0)->registerId);
    auto opB = args.at(1)->Read();
    return std::make_pair(opA, opB);
}
ConstantPair Processor::_Get_RM(std::vector<std::shared_ptr<Register>> args) const
{
    auto opA = args.at(0)->Read();
    auto opB = _DereferenceRegisterRead(args.at(1)->registerId);
    return std::make_pair(opA, opB);
}
ConstantPair Processor::_Get_MM(std::vector<std::shared_ptr<Register>> args) const
{
    auto opA = _DereferenceRegisterRead(args.at(0)->registerId);
    auto opB = _DereferenceRegisterRead(args.at(1)->registerId);
    return std::make_pair(opA, opB);
}

void Processor::_Base_ADD(ConstantPair values, std::shared_ptr<Register> dest)
{
    dest->Write(values.first + values.second);
}

void Processor::_Base_SUB(ConstantPair values, std::shared_ptr<Register> dest)
{
    dest->Write(values.first - values.second);
}
void Processor::_Base_MUL(ConstantPair values, std::shared_ptr<Register> dest)
{
    dest->Write(values.first * values.second);
}
void Processor::_Base_DIV(ConstantPair values, std::shared_ptr<Register> dest)
{
    dest->Write(values.first / values.second);
}
void Processor::_Base_AND(ConstantPair values, std::shared_ptr<Register> dest)
{
    dest->Write(values.first & values.second);
}
void Processor::_Base_OR(ConstantPair values, std::shared_ptr<Register> dest)
{
    dest->Write(values.first | values.second);
}
void Processor::_Base_XOR(ConstantPair values, std::shared_ptr<Register> dest)
{
    dest->Write(values.first ^ values.second);
}
void Processor::_Base_JZ(ConstantPair values)
{
    if (0 == values.first)
    {
        WriteRegister(RegisterId::RIP, values.second);
    }
}
void Processor::_Base_JNZ(ConstantPair values)
{
    if (0x0 != values.first)
    {
        WriteRegister(RegisterId::RIP, values.second);
    }
}

void Processor::ADD(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_ADD(vals, args.at(2));
}
void Processor::SUB(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_SUB(vals, args.at(2));
}
void Processor::MUL(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_MUL(vals, args.at(2));
}
void Processor::DIV(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_DIV(vals, args.at(2));
}
void Processor::AND(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_AND(vals, args.at(2));
}
void Processor::OR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_OR(vals, args.at(2));
}
void Processor::XOR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_XOR(vals, args.at(2));
}
void Processor::ADD_MR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MR(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_ADD(vals, racPtr);
}
void Processor::SUB_MR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MR(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_SUB(vals, racPtr);
}
void Processor::MUL_MR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MR(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_MUL(vals, racPtr);
}
void Processor::DIV_MR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MR(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_DIV(vals, racPtr);
}
void Processor::AND_MR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MR(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_AND(vals, racPtr);
}
void Processor::OR_MR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MR(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_OR(vals, racPtr);
}
void Processor::XOR_MR(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MR(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_XOR(vals, racPtr);
}
void Processor::ADD_RM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_ADD(vals, racPtr);
}
void Processor::SUB_RM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_SUB(vals, racPtr);
}
void Processor::MUL_RM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_MUL(vals, racPtr);
}
void Processor::DIV_RM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_DIV(vals, racPtr);
}
void Processor::AND_RM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_AND(vals, racPtr);
}
void Processor::OR_RM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_OR(vals, racPtr);
}
void Processor::XOR_RM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_XOR(vals, racPtr);
}
void Processor::ADD_MM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_ADD(vals, racPtr);
}
void Processor::SUB_MM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_SUB(vals, racPtr);
}
void Processor::MUL_MM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_MUL(vals, racPtr);
}
void Processor::DIV_MM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_DIV(vals, racPtr);
}
void Processor::AND_MM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_AND(vals, racPtr);
}
void Processor::OR_MM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_OR(vals, racPtr);
}
void Processor::XOR_MM(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_MM(args);
    std::shared_ptr racPtr = std::make_shared<Register>(_registers.at(RegisterId::RAC));
    _Base_XOR(vals, racPtr);
}
void Processor::JZ(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_JZ(vals);
}
void Processor::JNZ(std::vector<std::shared_ptr<Register>> args)
{
    auto vals = _Get_RR(args);
    _Base_JNZ(vals);
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
    auto opA = args.at(0)->Read();
    auto opB = args.at(1)->Read();
    bool isBitOn = opB & (1 << opA);
    uint16_t currentFlags = _registers.at(RegisterId::RFL).Read();
    if (isBitOn)
    {
        _registers.at(RegisterId::RFL).Write(currentFlags | static_cast<uint16_t>(FlagsRegister::Zero));
    }
    else
    {
        _registers.at(RegisterId::RFL).Write(currentFlags & ~static_cast<uint16_t>(FlagsRegister::Zero));
    }
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
}
void Processor::STOP(std::vector<std::shared_ptr<Register>> args)
{
    // args is not really used, but works for our table of ptrs to funcs.
    _instructionStatus = InstructionCycle::Halted;
}
void Processor::JZ_MR(std::vector<std::shared_ptr<Register>> args)
{
    _Base_JZ(_Get_MR(args));
}
void Processor::JNZ_MR(std::vector<std::shared_ptr<Register>> args)
{
    _Base_JNZ(_Get_MR(args));
}
void Processor::JNZ_MM(std::vector<std::shared_ptr<Register>> args)
{
    _Base_JNZ(_Get_MM(args));
}
void Processor::JZ_RM(std::vector<std::shared_ptr<Register>> args)
{
    _Base_JZ(_Get_RM(args));
}
void Processor::JZ_MM(std::vector<std::shared_ptr<Register>> args)
{
    _Base_JZ(_Get_MM(args));
}
void Processor::JNZ_RM(std::vector<std::shared_ptr<Register>> args)
{
    _Base_JNZ(_Get_RM(args));
}
void Processor::MOV_RM(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0)->Read();
    _DereferenceRegisterWrite(args.at(1)->registerId, opA);
}
void Processor::MOV_MR(std::vector<std::shared_ptr<Register>> args)
{
    uint16_t opA = _DereferenceRegisterRead(args.at(0)->registerId);
    auto opB = args.at(1);
    opB->Write(opA);
}
void Processor::MOV_MM(std::vector<std::shared_ptr<Register>> args)
{
    uint16_t opA = _DereferenceRegisterRead(args.at(0)->registerId);
    _DereferenceRegisterWrite(args.at(1)->registerId, opA);
}
void Processor::TSTB_M(std::vector<std::shared_ptr<Register>> args)
{
    auto opA = args.at(0)->Read();
    auto opB = _DereferenceRegisterRead(args.at(1)->registerId);
    bool isBitOn = opB & (1 << opA);
    uint16_t currentFlags = _registers.at(RegisterId::RFL).Read();
    if (isBitOn)
    {
        _registers.at(RegisterId::RFL).Write(currentFlags | static_cast<uint16_t>(FlagsRegister::Zero));
    }
    else
    {
        _registers.at(RegisterId::RFL).Write(currentFlags & ~static_cast<uint16_t>(FlagsRegister::Zero));
    }
}
void Processor::SETZ_M(std::vector<std::shared_ptr<Register>> args)
{
    _DereferenceRegisterWrite(args.at(0)->registerId, 0x0);
}
void Processor::SETO_M(std::vector<std::shared_ptr<Register>> args)
{
    _DereferenceRegisterWrite(args.at(0)->registerId, 0xffff);
}
void Processor::SET_M(std::vector<std::shared_ptr<Register>> args)
{
    _DereferenceRegisterWrite(args.at(0)->registerId, _2wordOperand);
}
void Processor::PUSH_M(std::vector<std::shared_ptr<Register>> args)
{
    auto derefA = _DereferenceRegisterRead(args.at(0)->registerId);
    auto &RSP = _registers.at(RegisterId::RSP);
    _DereferenceRegisterWrite(RSP.registerId, derefA);

    RSP.Write(RSP.Read() + 2);
}
void Processor::POP_M(std::vector<std::shared_ptr<Register>> args)
{
    auto &RSP = _registers.at(RegisterId::RSP);
    RSP.Write(RSP.Read() - 2);

    _DereferenceRegisterWrite(args.at(0)->registerId, _DereferenceRegisterRead(RSP.registerId));
}
void Processor::NOT_M(std::vector<std::shared_ptr<Register>> args)
{
    auto derefA = _DereferenceRegisterRead(args.at(0)->registerId);
    derefA = ~derefA;
    _DereferenceRegisterWrite(args.at(0)->registerId, derefA);
}
void Processor::SHFR_M(std::vector<std::shared_ptr<Register>> args)
{
    auto derefA = _DereferenceRegisterRead(args.at(0)->registerId);
    _DereferenceRegisterWrite(args.at(0)->registerId, derefA >> 1);
}
void Processor::SHFL_M(std::vector<std::shared_ptr<Register>> args)
{
    auto derefA = _DereferenceRegisterRead(args.at(0)->registerId);
    _DereferenceRegisterWrite(args.at(0)->registerId, derefA << 1);
}
void Processor::INC_M(std::vector<std::shared_ptr<Register>> args)
{
    auto derefA = _DereferenceRegisterRead(args.at(0)->registerId);
    _DereferenceRegisterWrite(args.at(0)->registerId, derefA + 1);
}
void Processor::DEC_M(std::vector<std::shared_ptr<Register>> args)
{
    auto derefA = _DereferenceRegisterRead(args.at(0)->registerId);
    _DereferenceRegisterWrite(args.at(0)->registerId, derefA - 1);
}
