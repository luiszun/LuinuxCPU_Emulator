#pragma once
#include "common.h"

// TODO: Consider implementing uninitialized memory checks
template <typename TAddressSpace> class Memory
{
  public:
    Memory(TAddressSpace size)
    {
        _memory.resize(size);
    }

    uint8_t Read(TAddressSpace address) const
    {
        assert(address < _memory.size());
        return _memory.at(address);
    }

    void Write(TAddressSpace address, uint8_t value)
    {
        assert(address < _memory.size());
        _memory[address] = value;
    }

    size_t Size() const
    {
        return _memory.size();
    }

  protected:
    std::vector<uint8_t> _memory;
    void _ValidateAddress(TAddressSpace address)
    {
        if (!(address < _memory.size))
        {
            throw std::out_of_range("Used address is out of range");
        }
    }
};
