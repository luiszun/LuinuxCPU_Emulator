#include "processor.h"
#include "common.h"

uint16_t Processor::_ReadMemoryWord(Memory16 &memory, uint16_t address) const
{
    uint16_t value = (memory.Read(address) << 8);
    value |= memory.Read(address + 1);
    return value;
}

void Processor::_WriteMemoryWord(Memory16 &memory, uint16_t address, uint16_t value)
{
    memory.Write(address, static_cast<uint8_t>(value >> 8));
    memory.Write(address + 1, static_cast<uint8_t>(value & 0x00ff));
}

void Processor::WriteRegister(RegisterId reg, uint16_t value)
{
    _registers.at(reg).Write(value);
}

uint16_t Processor::ReadRegister(RegisterId reg) const
{
    return _registers.at(reg).Read();
}

void Processor::_FetchInstruction()
{
    uint16_t ripVal = ReadRegister(RegisterId::RIP);
    _fetchedInstruction = _ReadMemoryWord(_programMemory, ripVal);
    WriteRegister(RegisterId::RIP, ripVal + 1);
}

void Processor::_DecodeInstruction()
{
    if ((_decodedOpCode != OpCodeId::INVALID_INSTR) || (_instructionArgs.size() > 0))
    {
        throw std::runtime_error("Decoding new instruction with previous exec cycle unfinished.");
    }
    _decodedOpCode = static_cast<OpCodeId>(_fetchedInstruction >> 12);
}

void Processor::_ExecuteInstruction()
{
}

void Processor::PerformExecutionCycle()
{
}

Processor::Processor(std::string diskFilename)
    : _intrMem(InternalMemorySize), _mainMemory(0x10000), _programMemory(0x10000, diskFilename)
{
    static_assert(static_cast<uint8_t>(RegisterId::RAC) == 0);
    for (auto i = RegisterId::RAC; i != RegisterId::END_OF_REGLIST; i = RegisterId(static_cast<uint8_t>(i) + 1))
    {
        _registers.insert({i, Register(static_cast<uint8_t>(i) * uint8_t{2}, _intrMem)});
    }

    WriteRegister(RegisterId::RSP, RSP_DefaultAddress);
    WriteRegister(RegisterId::RIP, 0);
}
