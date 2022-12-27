#include "processor.h"

uint16_t Processor::ReadMemoryWord(Memory16 &memory, uint16_t address) const
{
    uint16_t value = (memory.Read(address) << 8);
    value |= memory.Read(address + 1);
    return value;
}

void Processor::WriteMemoryWord(Memory16 &memory, uint16_t address, uint16_t value)
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