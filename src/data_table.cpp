#include "assembler.h"
#include "opcode.h"

std::unordered_map<std::string, OpCode> opCodeTable = {

    {"ADD", OpCode{OpCodeId::ADD, 0x0, 3}},         {"SUB", OpCode{OpCodeId::SUB, 0x1, 3}},
    {"MUL", OpCode{OpCodeId::MUL, 0x2, 3}},         {"DIV", OpCode{OpCodeId::DIV, 0x3, 3}},
    {"AND", OpCode{OpCodeId::AND, 0x4, 3}},         {"OR", OpCode{OpCodeId::OR, 0x5, 3}},
    {"XOR", OpCode{OpCodeId::XOR, 0x6, 3}},         {"JZ", OpCode{OpCodeId::JZ, 0x70, 2}},
    {"JNZ", OpCode{OpCodeId::JNZ, 0x71, 2}},        {"MOV", OpCode{OpCodeId::MOV, 0x72, 2}},
    {"LOAD", OpCode{OpCodeId::LOAD, 0x73, 2}},      {"STOR", OpCode{OpCodeId::STOR, 0x74, 2}},
    {"TSTB", OpCode{OpCodeId::TSTB, 0x75, 2}},      {"SETZ", OpCode{OpCodeId::SETZ, 0x760, 1}},
    {"SETO", OpCode{OpCodeId::SETO, 0x761, 1}},     {"SET", OpCode{OpCodeId::SET, 0x762, 1}},
    {"PUSH", OpCode{OpCodeId::PUSH, 0x763, 1}},     {"POP", OpCode{OpCodeId::POP, 0x764, 1}},
    {"NOT", OpCode{OpCodeId::NOT, 0x765, 1}},       {"SHFR", OpCode{OpCodeId::SHFR, 0x766, 1}},
    {"SHFL", OpCode{OpCodeId::SHFL, 0x767, 1}},     {"INC", OpCode{OpCodeId::INC, 0x768, 1}},
    {"DEC", OpCode{OpCodeId::DEC, 0x963, 1}},       {"NOP", OpCode{OpCodeId::NOP, 0x7690, 0}},
    {"STOP", OpCode{OpCodeId::STOP, 0x7691, 0}},    {"ADD_RM", OpCode{OpCodeId::ADD_RM, 0x77, 2}},
    {"ADD_MR", OpCode{OpCodeId::ADD_MR, 0x78, 2}},  {"ADD_MM", OpCode{OpCodeId::ADD_MM, 0x79, 2}},
    {"SUB_RM", OpCode{OpCodeId::SUB_RM, 0x7a, 2}},  {"SUB_MR", OpCode{OpCodeId::SUB_MR, 0x7b, 2}},
    {"SUB_MM", OpCode{OpCodeId::SUB_MM, 0x7c, 2}},  {"MUL_RM", OpCode{OpCodeId::MUL_RM, 0x7d, 2}},
    {"MUL_MR", OpCode{OpCodeId::MUL_MR, 0x7e, 2}},  {"MUL_MM", OpCode{OpCodeId::MUL_MM, 0x7f, 2}},
    {"DIV_RM", OpCode{OpCodeId::DIV_RM, 0x80, 2}},  {"DIV_MR", OpCode{OpCodeId::DIV_MR, 0x81, 2}},
    {"DIV_MM", OpCode{OpCodeId::DIV_MM, 0x82, 2}},  {"AND_RM", OpCode{OpCodeId::AND_RM, 0x83, 2}},
    {"AND_MR", OpCode{OpCodeId::AND_MR, 0x84, 2}},  {"AND_MM", OpCode{OpCodeId::AND_MM, 0x85, 2}},
    {"OR_RM", OpCode{OpCodeId::OR_RM, 0x86, 2}},    {"OR_MR", OpCode{OpCodeId::OR_MR, 0x87, 2}},
    {"OR_MM", OpCode{OpCodeId::OR_MM, 0x88, 2}},    {"XOR_RM", OpCode{OpCodeId::XOR_RM, 0x89, 2}},
    {"XOR_MR", OpCode{OpCodeId::XOR_MR, 0x8a, 2}},  {"XOR_MM", OpCode{OpCodeId::XOR_MM, 0x8b, 2}},
    {"JZ_RM", OpCode{OpCodeId::JZ_RM, 0x8c, 2}},    {"JZ_MR", OpCode{OpCodeId::JZ_MR, 0x8d, 2}},
    {"JZ_MM", OpCode{OpCodeId::JZ_MM, 0x8e, 2}},    {"JNZ_RM", OpCode{OpCodeId::JNZ_RM, 0x8f, 2}},
    {"JNZ_MR", OpCode{OpCodeId::JNZ_MR, 0x90, 2}},  {"JNZ_MM", OpCode{OpCodeId::JNZ_MM, 0x91, 2}},
    {"MOV_RM", OpCode{OpCodeId::MOV_RM, 0x92, 2}},  {"MOV_MR", OpCode{OpCodeId::MOV_MR, 0x93, 2}},
    {"MOV_MM", OpCode{OpCodeId::MOV_MM, 0x94, 2}},  {"TSTB_M", OpCode{OpCodeId::TSTB_M, 0x95, 2}},
    {"SETZ_M", OpCode{OpCodeId::SETZ_M, 0x76a, 1}}, {"SETO_M", OpCode{OpCodeId::SETO_M, 0x76b, 1}},
    {"SET_M", OpCode{OpCodeId::SET_M, 0x76c, 1}},   {"PUSH_M", OpCode{OpCodeId::PUSH_M, 0x76d, 1}},
    {"POP_M", OpCode{OpCodeId::POP_M, 0x76e, 1}},   {"NOT_M", OpCode{OpCodeId::NOT_M, 0x76f, 1}},
    {"SHFR_M", OpCode{OpCodeId::SHFR_M, 0x960, 1}}, {"SHFL_M", OpCode{OpCodeId::SHFL_M, 0x961, 1}},
    {"INC_M", OpCode{OpCodeId::INC_M, 0x962, 1}},   {"DEC_M", OpCode{OpCodeId::DEC_M, 0x964, 1}},
};

std::unordered_map<std::string, RegisterId> registerMap = {
    {"RAC", RegisterId::RAC}, {"RFL", RegisterId::RFL}, {"RIP", RegisterId::RIP}, {"RSP", RegisterId::RSP},
    {"RBP", RegisterId::RBP}, {"R0", RegisterId::R0},   {"R1", RegisterId::R1},   {"R2", RegisterId::R2},
    {"R3", RegisterId::R3},   {"R4", RegisterId::R4},   {"R5", RegisterId::R5},   {"R6", RegisterId::R6},
    {"R7", RegisterId::R7},   {"R8", RegisterId::R8},   {"R9", RegisterId::R9},   {"R10", RegisterId::R10},
    {"R11", RegisterId::R11}};