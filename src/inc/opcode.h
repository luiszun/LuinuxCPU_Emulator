#include <array>
#include <string>
#include <tuple>
#include <unordered_map>

struct OpCode
{
    std::string name;
    uint16_t opCode;
    uint8_t argCount;
};

std::unordered_map<std::string, OpCode> opCodeTable = {

    {"ADD", OpCode{"ADD", 0x0, 3}},         {"SUB", OpCode{"SUB", 0x1, 3}},
    {"MUL", OpCode{"MUL", 0x2, 3}},         {"DIV", OpCode{"DIV", 0x3, 3}},
    {"AND", OpCode{"AND", 0x4, 3}},         {"OR", OpCode{"OR", 0x5, 3}},
    {"XOR", OpCode{"XOR", 0x6, 3}},         {"JZ", OpCode{"JZ", 0x70, 2}},
    {"JNZ", OpCode{"JNZ", 0x71, 2}},        {"MOV", OpCode{"MOV", 0x72, 2}},
    {"LOAD", OpCode{"LOAD", 0x73, 2}},      {"STOR", OpCode{"STOR", 0x74, 2}},
    {"TSTB", OpCode{"TSTB", 0x75, 2}},      {"SETZ", OpCode{"SETZ", 0x760, 1}},
    {"SETO", OpCode{"SETO", 0x761, 1}},     {"SET", OpCode{"SET", 0x762, 1}},
    {"PUSH", OpCode{"PUSH", 0x763, 1}},     {"POP", OpCode{"POP", 0x764, 1}},
    {"NOT", OpCode{"NOT", 0x765, 1}},       {"SHFR", OpCode{"SHFR", 0x766, 1}},
    {"SHFL", OpCode{"SHFL", 0x767, 1}},     {"INC", OpCode{"INC", 0x768, 1}},
    {"NOP", OpCode{"NOP", 0x7690, 0}},      {"STOP", OpCode{"STOP", 0x7691, 0}},
    {"ADD_RM", OpCode{"ADD_RM", 0x77, 2}},  {"ADD_MR", OpCode{"ADD_MR", 0x78, 2}},
    {"ADD_MM", OpCode{"ADD_MM", 0x79, 2}},  {"SUB_RM", OpCode{"SUB_RM", 0x7a, 2}},
    {"SUB_MR", OpCode{"SUB_MR", 0x7b, 2}},  {"SUB_MM", OpCode{"SUB_MM", 0x7c, 2}},
    {"MUL_RM", OpCode{"MUL_RM", 0x7d, 2}},  {"MUL_MR", OpCode{"MUL_MR", 0x7e, 2}},
    {"MUL_MM", OpCode{"MUL_MM", 0x7f, 2}},  {"DIV_RM", OpCode{"DIV_RM", 0x80, 2}},
    {"DIV_MR", OpCode{"DIV_MR", 0x81, 2}},  {"DIV_MM", OpCode{"DIV_MM", 0x82, 2}},
    {"AND_RM", OpCode{"AND_RM", 0x83, 2}},  {"AND_MR", OpCode{"AND_MR", 0x84, 2}},
    {"AND_MM", OpCode{"AND_MM", 0x85, 2}},  {"OR_RM", OpCode{"OR_RM", 0x86, 2}},
    {"OR_MR", OpCode{"OR_MR", 0x87, 2}},    {"OR_MM", OpCode{"OR_MM", 0x88, 2}},
    {"XOR_RM", OpCode{"XOR_RM", 0x89, 2}},  {"XOR_MR", OpCode{"XOR_MR", 0x8a, 2}},
    {"XOR_MM", OpCode{"XOR_MM", 0x8b, 2}},  {"JZ_RM", OpCode{"JZ_RM", 0x8c, 2}},
    {"JZ_MR", OpCode{"JZ_MR", 0x8d, 2}},    {"JZ_MM", OpCode{"JZ_MM", 0x8e, 2}},
    {"JNZ_RM", OpCode{"JNZ_RM", 0x8f, 2}},  {"JNZ_MR", OpCode{"JNZ_MR", 0x90, 2}},
    {"JNZ_MM", OpCode{"JNZ_MM", 0x91, 2}},  {"MOV_RM", OpCode{"MOV_RM", 0x92, 2}},
    {"MOV_MR", OpCode{"MOV_MR", 0x93, 2}},  {"MOV_MM", OpCode{"MOV_MM", 0x94, 2}},
    {"TSTB_M", OpCode{"TSTB_M", 0x95, 2}},  {"SETZ_M", OpCode{"SETZ_M", 0x76a, 1}},
    {"SETO_M", OpCode{"SETO_M", 0x76b, 1}}, {"SET_M", OpCode{"SET_M", 0x76c, 1}},
    {"PUSH_M", OpCode{"PUSH_M", 0x76d, 1}}, {"POP_M", OpCode{"POP_M", 0x76e, 1}},
    {"NOT_M", OpCode{"NOT_M", 0x76f, 1}},   {"SHFR_M", OpCode{"SHFR_M", 0x960, 1}},
    {"SHFL_M", OpCode{"SHFL_M", 0x961, 1}}, {"INC_M", OpCode{"INC_M", 0x962, 1}},
};
