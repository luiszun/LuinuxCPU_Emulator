#pragma once
#include "common.h"

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