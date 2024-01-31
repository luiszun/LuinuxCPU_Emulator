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

    uint8_t Read8(TAddressSpace address) const
    {
        _ValidateAddress(address);
        return _memory.at(address);
    }

    void Write8(TAddressSpace address, uint8_t value)
    {
        _ValidateAddress(address);
        _memory[address] = value;
    }

    void Write(TAddressSpace address, uint16_t value)
    {
        Write8(address, static_cast<uint8_t>(value >> 8));
        Write8(address + 1, static_cast<uint8_t>(value & 0x00ff));
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
        _FlushIn();
    }

    ~NVMemory()
    {
        _outFile.open(_filename, std::ios::binary | std::ios::trunc | std::ios::out);
        _outFile.write(reinterpret_cast<char *>(&(this->_memory[0])), this->_memory.size());
        _outFile.close();
    }

    void Flush()
    {
        _FlushOut();
        _FlushIn();
    }

  private:
    void _FlushOut()
    {
        _outFile.open(_filename, std::ios::binary | std::ios::trunc | std::ios::out);
        _outFile.write(reinterpret_cast<char *>(&(this->_memory[0])), this->_memory.size());
        _outFile.close();
    }

    void _FlushIn()
    {
        _inFile.open(_filename, std::ios::binary | std::ios::in);
        assert(_inFile.is_open());

        _inFile.read(reinterpret_cast<char *>(&(this->_memory[0])), this->_memory.size());
        _inFile.close();
    }

    std::fstream _inFile;
    std::fstream _outFile;
    std::string _filename;
};