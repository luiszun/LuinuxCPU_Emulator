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

enum class FlagsRegister : uint16_t
{
    Zero = 0x0001,
    Carry = 0x0002,
    Negative = 0x0004,
    Trap = 0x0008,
    Reserved = 0x0010,
    StackOverflow = 0x0020,
    Exception = 0x0040
};

extern const std::unordered_map<std::string, RegisterId> registerMap;

class Register
{
  public:
    Register(uint16_t address, Memory8 &memory, RegisterId inRegisterId)
        : _address(address), _memory(memory), registerId(inRegisterId)
    {
    }

    uint16_t Read() const
    {
        uint16_t value = (_memory.Read8(_address) << 8);
        value |= _memory.Read8(_address + 1);
        return value;
    }
    void Write(uint16_t value)
    {
        _memory.Write16(_address, value);
    }

    const RegisterId registerId;

  protected:
    uint8_t _address;
    Memory8 &_memory;
};

struct FlagsUnion
{
    unsigned Zero : 1;
    unsigned Carry : 1;
    unsigned Negative : 1;
    unsigned Trap : 1;
    unsigned Reserved : 1;
    unsigned StackOverflow : 1;
    unsigned Exception : 1;
};

union FlagsObject {
    FlagsUnion flags;
    uint16_t value;

    FlagsObject(uint16_t v) : value(v){}
};
