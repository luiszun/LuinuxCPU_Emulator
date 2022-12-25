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
    R11
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

extern std::unordered_map<std::string, RegisterId> registerMap;

class Register
{
  public:
    Register(uint16_t address) : _address(address)
    {
    }

    uint16_t Read()
    {
        uint16_t value = (_memory->Read(_address) << 8);
        value |= _memory->Read(_address + 1);
        return value;
    }
    void Write(uint16_t value)
    {
        _memory->Write(_address, static_cast<uint8_t>(value >> 8));
        _memory->Write(_address + 1, static_cast<uint8_t>(value & 0x00ff));
    }

  protected:
    uint8_t _address;
    std::shared_ptr<Memory8> _memory;
};