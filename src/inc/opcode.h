#pragma once
#include "common.h"

enum class OpCodeId
{
    ADD = 0,
    SUB,
    MUL,
    DIV,
    SMUL,
    SDIV,
    AND,
    OR,
    XOR,
    JZ,
    JNZ,
    MOV,
    JE,
    JNE,
    LOAD,
    STOR,
    TSTB,
    SETZ,
    SETO,
    SET,
    PUSH,
    POP,
    NOT,
    SHFR,
    SHFL,
    INC,
    DEC,
    NOP,
    STOP,
    TRAP,
    SWM,
    JMP,
    INVALID_INSTR
};

struct OpCode
{
    uint16_t opCode;
    uint8_t argCount;
};

extern const std::unordered_map<std::string, OpCodeId> mnemonicTable;
extern const std::unordered_map<OpCodeId, std::string> opCodeMnemonicTable;
extern const std::unordered_map<OpCodeId, OpCode> opCodeTable;
extern const std::unordered_map<uint16_t, OpCodeId> opCodeValuesTable;
extern const std::array<std::string_view, 16> registerNameTable;