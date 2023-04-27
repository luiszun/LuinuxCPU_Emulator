#pragma once
#include "common.h"

// TODO: Consider implementing uninitialized memory checks
template <typename TAddressSpace> class Memory
{
  public:
    Memory(size_t size)
    {
        _memory.reserve(size);
        _memory.resize(size, 0);
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

    void Write(TAddressSpace address, const unsigned char *shellCode, size_t size)
    {
        _ValidateAddress(address);
        std::memcpy(&_memory[address], shellCode, size);
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

template <typename TAddressSpace> class NVMemory : public Memory<TAddressSpace>
{
  public:
    NVMemory(size_t size, std::string filename) : Memory<TAddressSpace>(size), _filename(filename)
    {
        _diskFile.open(_filename, std::ios::binary | std::ios::in);
        _diskFile.read(reinterpret_cast<char *>(&(this->_memory[0])), this->_memory.size());
        _diskFile.close();
    }

    ~NVMemory()
    {
        _diskFile.open(_filename, std::ios::binary | std::ios::trunc | std::ios::out);
        _diskFile.write(reinterpret_cast<char *>(&(this->_memory[0])), this->_memory.size());
        _diskFile.close();
    }

  private:
    std::fstream _diskFile;
    std::string _filename;
};