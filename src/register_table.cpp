#include "assembler.h"

const std::unordered_map<std::string, RegisterId> registerMap = {
    {"RAC", RegisterId::RAC},
    {"RFL", RegisterId::RFL},
    {"RIP", RegisterId::RIP},
    {"RSP", RegisterId::RSP},
    {"RBP", RegisterId::RBP},
    {"R0", RegisterId::R0},
    {"R1", RegisterId::R1},
    {"R2", RegisterId::R2},
    {"R3", RegisterId::R3},
    {"R4", RegisterId::R4},
    {"R5", RegisterId::R5},
    {"R6", RegisterId::R6},
    {"R7", RegisterId::R7},
    {"R8", RegisterId::R8},
    {"R9", RegisterId::R9},
    {"R10", RegisterId::R10},
};

const std::array<std::string_view, 16> registerNameTable = {
    "RAC",
    "RFL",
    "RIP",
    "RSP",
    "RBP",
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
};
