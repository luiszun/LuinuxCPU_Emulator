#pragma once
#include "memory.h"
#include "register.h"

// 256 bytes of internal memory, used for 8x register banks
constexpr size_t InternalMemorySize = 256;

using Memory16 = Memory<uint16_t>;
using NVMemory16 = NVMemory<uint16_t>;
using Memory8 = Memory<uint8_t>;

class Processor
{
  public:
    Processor(std::string diskFilename)
        : _intrMem(InternalMemorySize), _mainMemory(0x10000), _programMemory(0x10000, diskFilename)
    {
        static_assert(static_cast<uint8_t>(RegisterId::RAC) == 0);
        for (auto i = RegisterId::RAC; i != RegisterId::END_OF_REGLIST; i = RegisterId(static_cast<uint8_t>(i) + 1))
        {
            _registers.insert({i, Register(static_cast<uint8_t>(i) * uint8_t{2}, _intrMem)});
        }
    }

    void WriteRegister(RegisterId reg, uint16_t value);
    uint16_t ReadRegister(RegisterId reg) const;

    // TODO:
    // fetch
    // decode
    // execute
    // alu
    // flags updates
    // pause

  protected:
    uint16_t ReadMemoryWord(Memory16 &memory, uint16_t address) const;
    void WriteMemoryWord(Memory16 &memory, uint16_t address, uint16_t value);

    NVMemory16 _programMemory;
    Memory16 _mainMemory;
    Memory8 _intrMem;
    std::unordered_map<RegisterId, Register> _registers;
};
