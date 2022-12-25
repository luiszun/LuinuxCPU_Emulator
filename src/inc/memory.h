#pragma once
#include "common.h"

// TODO: Consider implementing uninitialized memory checks
template <typename TAddressSpace> class Memory
{
  public:
    Memory(size_t size)
    {
        _memory.resize(size);
    }

    uint8_t Read(TAddressSpace address) const
    {
        _ValidateAddress(address);
        return _memory.at(address);
    }

    void Write(TAddressSpace address, uint8_t value)
    {
        _ValidateAddress(address);
        _memory[address] = value;
    }

    size_t Size() const
    {
        return _memory.size();
    }

  protected:
    std::vector<uint8_t> _memory;
    void _ValidateAddress(TAddressSpace address) const
    {
        if (!(address < _memory.size()))
        {
            throw std::out_of_range("Used address is out of range");
        }
    }
};
