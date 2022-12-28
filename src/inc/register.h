#pragma once
#include "common.h"
#include "memory.h"

using Memory8 = Memory<uint8_t>;

enum class RegisterId : uint8_t
{
    RAC = 0,
    RFL,
    RIP,
    RSP,
    RBP,
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    END_OF_REGLIST
};

enum class FlagsRegister
{
    Zero = 0,
    Carry,
    Negative,
    Trap,
    Reserved,
    StackOverflow
};

extern const std::unordered_map<std::string, RegisterId> registerMap;

class Register
{
  public:
    Register(uint16_t address, Memory8 &memory) : _address(address), _memory(memory)
    {
    }

    uint16_t Read() const
    {
        uint16_t value = (_memory.Read(_address) << 8);
        value |= _memory.Read(_address + 1);
        return value;
    }
    void Write(uint16_t value)
    {
        _memory.Write(_address, static_cast<uint8_t>(value >> 8));
        _memory.Write(_address + 1, static_cast<uint8_t>(value & 0x00ff));
    }

  protected:
    uint8_t _address;
    Memory8 &_memory;
};