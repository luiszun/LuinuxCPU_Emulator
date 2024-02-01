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
    // Let's create a table of function pointers to the instructions. That way we only reference those by OpCodeId
    typedef void (Processor::*OpFunction)(std::vector<std::shared_ptr<Register>>);
    static const std::unordered_map<OpCodeId, OpFunction> opCodeFunctionTable = {
        {OpCodeId::ADD, &Processor::ADD}, {OpCodeId::SUB, &Processor::SUB}, {OpCodeId::MUL, &Processor::MUL}};

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


// TODO: Remove all these comments
// ALU operations ADD, SUB, MIL, DIV, AND, OR, XOR, NOT
// ALU-masked ops - INC, DEC
// Branching ops - JZ, JNZ
// Specific tasks - SET, SETZ, SETO, TSTB, PUSH, POP, NOP, STOP
// Data movement - MOV, LOAD, STOR
/*
    {OpCodeId::DIV, OpCode{0x3, 3}},      {OpCodeId::AND, OpCode{0x4, 3}},     {OpCodeId::OR, OpCode{0x5, 3}},
    {OpCodeId::XOR, OpCode{0x6, 3}},      {OpCodeId::JZ, OpCode{0x70, 2}},     {OpCodeId::JNZ, OpCode{0x71, 2}},
    {OpCodeId::MOV, OpCode{0x72, 2}},     {OpCodeId::LOAD, OpCode{0x73, 2}},   {OpCodeId::STOR, OpCode{0x74, 2}},
    {OpCodeId::TSTB, OpCode{0x75, 2}},    {OpCodeId::SETZ, OpCode{0x760, 1}},  {OpCodeId::SETO, OpCode{0x761, 1}},
    {OpCodeId::SET, OpCode{0x762, 1}},    {OpCodeId::PUSH, OpCode{0x763, 1}},  {OpCodeId::POP, OpCode{0x764, 1}},
    {OpCodeId::NOT, OpCode{0x765, 1}},    {OpCodeId::SHFR, OpCode{0x766, 1}},  {OpCodeId::SHFL, OpCode{0x767, 1}},
    {OpCodeId::INC, OpCode{0x768, 1}},    {OpCodeId::DEC, OpCode{0x963, 1}},   {OpCodeId::NOP, OpCode{0x7690, 0}},
    {OpCodeId::STOP, OpCode{0x7691, 0}},  {OpCodeId::ADD_RM, OpCode{0x77, 2}}, {OpCodeId::ADD_MR, OpCode{0x78, 2}},
    {OpCodeId::ADD_MM, OpCode{0x79, 2}},  {OpCodeId::SUB_RM, OpCode{0x7a, 2}}, {OpCodeId::SUB_MR, OpCode{0x7b, 2}},
    {OpCodeId::SUB_MM, OpCode{0x7c, 2}},  {OpCodeId::MUL_RM, OpCode{0x7d, 2}}, {OpCodeId::MUL_MR, OpCode{0x7e, 2}},
    {OpCodeId::MUL_MM, OpCode{0x7f, 2}},  {OpCodeId::DIV_RM, OpCode{0x80, 2}}, {OpCodeId::DIV_MR, OpCode{0x81, 2}},
    {OpCodeId::DIV_MM, OpCode{0x82, 2}},  {OpCodeId::AND_RM, OpCode{0x83, 2}}, {OpCodeId::AND_MR, OpCode{0x84, 2}},
    {OpCodeId::AND_MM, OpCode{0x85, 2}},  {OpCodeId::OR_RM, OpCode{0x86, 2}},  {OpCodeId::OR_MR, OpCode{0x87, 2}},
    {OpCodeId::OR_MM, OpCode{0x88, 2}},   {OpCodeId::XOR_RM, OpCode{0x89, 2}}, {OpCodeId::XOR_MR, OpCode{0x8a, 2}},
    {OpCodeId::XOR_MM, OpCode{0x8b, 2}},  {OpCodeId::JZ_RM, OpCode{0x8c, 2}},  {OpCodeId::JZ_MR, OpCode{0x8d, 2}},
    {OpCodeId::JZ_MM, OpCode{0x8e, 2}},   {OpCodeId::JNZ_RM, OpCode{0x8f, 2}}, {OpCodeId::JNZ_MR, OpCode{0x90, 2}},
    {OpCodeId::JNZ_MM, OpCode{0x91, 2}},  {OpCodeId::MOV_RM, OpCode{0x92, 2}}, {OpCodeId::MOV_MR, OpCode{0x93, 2}},
    {OpCodeId::MOV_MM, OpCode{0x94, 2}},  {OpCodeId::TSTB_M, OpCode{0x95, 2}}, {OpCodeId::SETZ_M, OpCode{0x76a, 1}},
    {OpCodeId::SETO_M, OpCode{0x76b, 1}}, {OpCodeId::SET_M, OpCode{0x76c, 1}}, {OpCodeId::PUSH_M, OpCode{0x76d, 1}},
    {OpCodeId::POP_M, OpCode{0x76e, 1}},  {OpCodeId::NOT_M, OpCode{0x76f, 1}}, {OpCodeId::SHFR_M, OpCode{0x960, 1}},
    {OpCodeId::SHFL_M, OpCode{0x961, 1}}, {OpCodeId::INC_M, OpCode{0x962, 1}}, {OpCodeId::DEC_M, OpCode{0x964, 1}}};
    */

// TODO include this in the processor code?
void Processor::ADD(std::vector<std::shared_ptr<Register>> args)
{
    auto OpA = args.at(0);
    auto OpB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(OpA->Read() + OpB->Read());
}
void Processor::SUB(std::vector<std::shared_ptr<Register>> args)
{
    auto OpA = args.at(0);
    auto OpB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(OpA->Read() - OpB->Read());
}
void Processor::MUL(std::vector<std::shared_ptr<Register>> args)
{
    auto OpA = args.at(0);
    auto OpB = args.at(1);
    auto OpResult = args.at(2);

    OpResult->Write(OpA->Read() * OpB->Read());
}

void Processor::STOP(std::vector<std::shared_ptr<Register>> args)
{
    // args is not really used, but works for our table of ptrs to funcs.
    _instructionStatus == InstructionCycle::Halted;
    
}
