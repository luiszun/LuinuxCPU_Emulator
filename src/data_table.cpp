#include "assembler.h"
#include "opcode.h"

const std::unordered_map<OpCodeId, OpCode> opCodeTable = {
    {OpCodeId::ADD, OpCode{0x0, 3}},      {OpCodeId::SUB, OpCode{0x1, 3}},     {OpCodeId::MUL, OpCode{0x2, 3}},
    {OpCodeId::DIV, OpCode{0x3, 3}},      {OpCodeId::AND, OpCode{0x4, 3}},     {OpCodeId::OR, OpCode{0x5, 3}},
    {OpCodeId::XOR, OpCode{0x6, 3}},      {OpCodeId::JZ, OpCode{0x70, 2}},     {OpCodeId::JNZ, OpCode{0x71, 2}},
    {OpCodeId::MOV, OpCode{0x72, 2}},     {OpCodeId::LOAD, OpCode{0x73, 2}},   {OpCodeId::STOR, OpCode{0x74, 2}},
    {OpCodeId::TSTB, OpCode{0x75, 2}},    {OpCodeId::SETZ, OpCode{0x760, 1}},  {OpCodeId::SETO, OpCode{0x761, 1}},
    {OpCodeId::SET, OpCode{0x762, 1}},    {OpCodeId::PUSH, OpCode{0x763, 1}},  {OpCodeId::POP, OpCode{0x764, 1}},
    {OpCodeId::NOT, OpCode{0x765, 1}},    {OpCodeId::SHFR, OpCode{0x766, 1}},  {OpCodeId::SHFL, OpCode{0x767, 1}},
    {OpCodeId::INC, OpCode{0x768, 1}},    {OpCodeId::DEC, OpCode{0x963, 1}},   {OpCodeId::NOP, OpCode{0x7690, 0}},
    {OpCodeId::STOP, OpCode{0x7691, 0}},  {OpCodeId::ADD_RM, OpCode{0x77, 2}}, {OpCodeId::ADD_MR, OpCode{0x78, 2}},
    {OpCodeId::ADD_MM, OpCode{0x79, 2}},  {OpCodeId::SUB_RM, OpCode{0x7a, 2}}, {OpCodeId::SUB_MR, OpCode{0x7b, 2}},
    {OpCodeId::SUB_MM, OpCode{0x7c, 2}},  {OpCodeId::MUL_RM, OpCode{0x7d, 2}}, {OpCodeId::MUL_MR, OpCode{0x7e, 2}},
    {OpCodeId::MUL_MM, OpCode{0x7f, 2}},  {OpCodeId::DIV_RM, OpCode{0x80, 2}}, {OpCodeId::DIV_MR, OpCode{0x81, 2}},
    {OpCodeId::DIV_MM, OpCode{0x82, 2}},  {OpCodeId::AND_RM, OpCode{0x83, 2}}, {OpCodeId::AND_MR, OpCode{0x84, 2}},
    {OpCodeId::AND_MM, OpCode{0x85, 2}},  {OpCodeId::OR_RM, OpCode{0x86, 2}},  {OpCodeId::OR_MR, OpCode{0x87, 2}},
    {OpCodeId::OR_MM, OpCode{0x88, 2}},   {OpCodeId::XOR_RM, OpCode{0x89, 2}}, {OpCodeId::XOR_MR, OpCode{0x8a, 2}},
    {OpCodeId::XOR_MM, OpCode{0x8b, 2}},  {OpCodeId::JZ_RM, OpCode{0x8c, 2}},  {OpCodeId::JZ_MR, OpCode{0x8d, 2}},
    {OpCodeId::JZ_MM, OpCode{0x8e, 2}},   {OpCodeId::JNZ_RM, OpCode{0x8f, 2}}, {OpCodeId::JNZ_MR, OpCode{0x90, 2}},
    {OpCodeId::JNZ_MM, OpCode{0x91, 2}},  {OpCodeId::MOV_RM, OpCode{0x92, 2}}, {OpCodeId::MOV_MR, OpCode{0x93, 2}},
    {OpCodeId::MOV_MM, OpCode{0x94, 2}},  {OpCodeId::TSTB_M, OpCode{0x95, 2}}, {OpCodeId::SETZ_M, OpCode{0x76a, 1}},
    {OpCodeId::SETO_M, OpCode{0x76b, 1}}, {OpCodeId::SET_M, OpCode{0x76c, 1}}, {OpCodeId::PUSH_M, OpCode{0x76d, 1}},
    {OpCodeId::POP_M, OpCode{0x76e, 1}},  {OpCodeId::NOT_M, OpCode{0x76f, 1}}, {OpCodeId::SHFR_M, OpCode{0x960, 1}},
    {OpCodeId::SHFL_M, OpCode{0x961, 1}}, {OpCodeId::INC_M, OpCode{0x962, 1}}, {OpCodeId::DEC_M, OpCode{0x964, 1}}};

const std::unordered_map<std::string, OpCodeId> mnemonicTable = {
    {"ADD", OpCodeId::ADD},       {"SUB", OpCodeId::SUB},       {"MUL", OpCodeId::MUL},
    {"DIV", OpCodeId::DIV},       {"AND", OpCodeId::AND},       {"OR", OpCodeId::OR},
    {"XOR", OpCodeId::XOR},       {"JZ", OpCodeId::JZ},         {"JNZ", OpCodeId::JNZ},
    {"MOV", OpCodeId::MOV},       {"LOAD", OpCodeId::LOAD},     {"STOR", OpCodeId::STOR},
    {"TSTB", OpCodeId::TSTB},     {"SETZ", OpCodeId::SETZ},     {"SETO", OpCodeId::SETO},
    {"SET", OpCodeId::SET},       {"PUSH", OpCodeId::PUSH},     {"POP", OpCodeId::POP},
    {"NOT", OpCodeId::NOT},       {"SHFR", OpCodeId::SHFR},     {"SHFL", OpCodeId::SHFL},
    {"INC", OpCodeId::INC},       {"DEC", OpCodeId::DEC},       {"NOP", OpCodeId::NOP},
    {"STOP", OpCodeId::STOP},     {"ADD_RM", OpCodeId::ADD_RM}, {"ADD_MR", OpCodeId::ADD_MR},
    {"ADD_MM", OpCodeId::ADD_MM}, {"SUB_RM", OpCodeId::SUB_RM}, {"SUB_MR", OpCodeId::SUB_MR},
    {"SUB_MM", OpCodeId::SUB_MM}, {"MUL_RM", OpCodeId::MUL_RM}, {"MUL_MR", OpCodeId::MUL_MR},
    {"MUL_MM", OpCodeId::MUL_MM}, {"DIV_RM", OpCodeId::DIV_RM}, {"DIV_MR", OpCodeId::DIV_MR},
    {"DIV_MM", OpCodeId::DIV_MM}, {"AND_RM", OpCodeId::AND_RM}, {"AND_MR", OpCodeId::AND_MR},
    {"AND_MM", OpCodeId::AND_MM}, {"OR_RM", OpCodeId::OR_RM},   {"OR_MR", OpCodeId::OR_MR},
    {"OR_MM", OpCodeId::OR_MM},   {"XOR_RM", OpCodeId::XOR_RM}, {"XOR_MR", OpCodeId::XOR_MR},
    {"XOR_MM", OpCodeId::XOR_MM}, {"JZ_RM", OpCodeId::JZ_RM},   {"JZ_MR", OpCodeId::JZ_MR},
    {"JZ_MM", OpCodeId::JZ_MM},   {"JNZ_RM", OpCodeId::JNZ_RM}, {"JNZ_MR", OpCodeId::JNZ_MR},
    {"JNZ_MM", OpCodeId::JNZ_MM}, {"MOV_RM", OpCodeId::MOV_RM}, {"MOV_MR", OpCodeId::MOV_MR},
    {"MOV_MM", OpCodeId::MOV_MM}, {"TSTB_M", OpCodeId::TSTB_M}, {"SETZ_M", OpCodeId::SETZ_M},
    {"SETO_M", OpCodeId::SETO_M}, {"SET_M", OpCodeId::SET_M},   {"PUSH_M", OpCodeId::PUSH_M},
    {"POP_M", OpCodeId::POP_M},   {"NOT_M", OpCodeId::NOT_M},   {"SHFR_M", OpCodeId::SHFR_M},
    {"SHFL_M", OpCodeId::SHFL_M}, {"INC_M", OpCodeId::INC_M},   {"DEC_M", OpCodeId::DEC_M}};

const std::unordered_map<std::string, RegisterId> registerMap = {
    {"RAC", RegisterId::RAC}, {"RFL", RegisterId::RFL}, {"RIP", RegisterId::RIP}, {"RSP", RegisterId::RSP},
    {"RBP", RegisterId::RBP}, {"R0", RegisterId::R0},   {"R1", RegisterId::R1},   {"R2", RegisterId::R2},
    {"R3", RegisterId::R3},   {"R4", RegisterId::R4},   {"R5", RegisterId::R5},   {"R6", RegisterId::R6},
    {"R7", RegisterId::R7},   {"R8", RegisterId::R8},   {"R9", RegisterId::R9},   {"R10", RegisterId::R10},
    {"R11", RegisterId::R11}};