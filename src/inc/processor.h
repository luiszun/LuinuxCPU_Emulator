#pragma once
#include "memory.h"
#include "register.h"

// 128 bytes of internal memory, used for 4x register banks
constexpr size_t InternalMemorySize = 128;

using Memory16 = Memory<uint16_t>;
using Memory8 = Memory<uint8_t>;

class Processor
{
  public:
    Processor() : _intrMem(InternalMemorySize)
    {
        constexpr unsigned nRegistetrs = 16;
        for (unsigned i = 0; i < nRegistetrs; ++i)
        {
            _registers.push_back(Register(i * sizeof(uint16_t)));
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
    std::shared_ptr<Memory16> _mainMem;
    Memory8 _intrMem;
    std::vector<Register> _registers;
};
