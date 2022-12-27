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
    Processor() : _intrMem(InternalMemorySize), _mainMemory(0x10000)
    {
        constexpr unsigned nRegistetrs = 16;
        for (uint8_t i = 0; i < nRegistetrs; ++i)
        {
            _registers.push_back(Register(i * uint8_t{2}, _intrMem));
        }
    }

    // TODO:
    // fetch
    // decode
    // execute
    // alu
    // flags updates
    // pause

  protected:
    std::shared_ptr<NVMemory16> _programMemory;
    Memory16 _mainMemory;
    Memory8 _intrMem;
    std::vector<Register> _registers;
};
