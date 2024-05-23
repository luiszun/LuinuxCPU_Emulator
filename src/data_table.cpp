#include "assembler.h"
#include "opcode.h"

const std::unordered_map<OpCodeId, OpCode> opCodeTable = {
    {OpCodeId::ADD, OpCode{0x0, 3}},      {OpCodeId::SUB, OpCode{0x1, 3}},      {OpCodeId::MUL, OpCode{0x2, 3}},
    {OpCodeId::DIV, OpCode{0x3, 3}},      {OpCodeId::AND, OpCode{0x4, 3}},      {OpCodeId::OR, OpCode{0x5, 3}},
    {OpCodeId::XOR, OpCode{0x6, 3}},      {OpCodeId::JZ, OpCode{0x70, 2}},      {OpCodeId::JNZ, OpCode{0x71, 2}},
    {OpCodeId::MOV, OpCode{0x72, 2}},     {OpCodeId::TSTB, OpCode{0x75, 2}},    {OpCodeId::SETZ, OpCode{0x760, 1}},
    {OpCodeId::SETO, OpCode{0x761, 1}},   {OpCodeId::SET, OpCode{0x762, 1}},    {OpCodeId::PUSH, OpCode{0x763, 1}},
    {OpCodeId::POP, OpCode{0x764, 1}},    {OpCodeId::NOT, OpCode{0x765, 1}},    {OpCodeId::SHFR, OpCode{0x766, 1}},
    {OpCodeId::SHFL, OpCode{0x767, 1}},   {OpCodeId::INC, OpCode{0x768, 1}},    {OpCodeId::DEC, OpCode{0x963, 1}},
    {OpCodeId::NOP, OpCode{0x7690, 0}},   {OpCodeId::STOP, OpCode{0x7691, 0}},  {OpCodeId::ADD_RM, OpCode{0x77, 2}},
    {OpCodeId::ADD_MR, OpCode{0x78, 2}},  {OpCodeId::ADD_MM, OpCode{0x79, 2}},  {OpCodeId::SUB_RM, OpCode{0x7a, 2}},
    {OpCodeId::SUB_MR, OpCode{0x7b, 2}},  {OpCodeId::SUB_MM, OpCode{0x7c, 2}},  {OpCodeId::MUL_RM, OpCode{0x7d, 2}},
    {OpCodeId::MUL_MR, OpCode{0x7e, 2}},  {OpCodeId::MUL_MM, OpCode{0x7f, 2}},  {OpCodeId::DIV_RM, OpCode{0x80, 2}},
    {OpCodeId::DIV_MR, OpCode{0x81, 2}},  {OpCodeId::DIV_MM, OpCode{0x82, 2}},  {OpCodeId::AND_RM, OpCode{0x83, 2}},
    {OpCodeId::AND_MR, OpCode{0x84, 2}},  {OpCodeId::AND_MM, OpCode{0x85, 2}},  {OpCodeId::OR_RM, OpCode{0x86, 2}},
    {OpCodeId::OR_MR, OpCode{0x87, 2}},   {OpCodeId::OR_MM, OpCode{0x88, 2}},   {OpCodeId::XOR_RM, OpCode{0x89, 2}},
    {OpCodeId::XOR_MR, OpCode{0x8a, 2}},  {OpCodeId::XOR_MM, OpCode{0x8b, 2}},  {OpCodeId::JZ_RM, OpCode{0x8c, 2}},
    {OpCodeId::JZ_MR, OpCode{0x8d, 2}},   {OpCodeId::JZ_MM, OpCode{0x8e, 2}},   {OpCodeId::JNZ_RM, OpCode{0x8f, 2}},
    {OpCodeId::JNZ_MR, OpCode{0x90, 2}},  {OpCodeId::JNZ_MM, OpCode{0x91, 2}},  {OpCodeId::MOV_RM, OpCode{0x92, 2}},
    {OpCodeId::MOV_MR, OpCode{0x93, 2}},  {OpCodeId::MOV_MM, OpCode{0x94, 2}},  {OpCodeId::TSTB_M, OpCode{0x95, 2}},
    {OpCodeId::SETZ_M, OpCode{0x76a, 1}}, {OpCodeId::SETO_M, OpCode{0x76b, 1}}, {OpCodeId::SET_M, OpCode{0x76c, 1}},
    {OpCodeId::PUSH_M, OpCode{0x76d, 1}}, {OpCodeId::POP_M, OpCode{0x76e, 1}},  {OpCodeId::NOT_M, OpCode{0x76f, 1}},
    {OpCodeId::SHFR_M, OpCode{0x960, 1}}, {OpCodeId::SHFL_M, OpCode{0x961, 1}}, {OpCodeId::INC_M, OpCode{0x962, 1}},
    {OpCodeId::DEC_M, OpCode{0x964, 1}},  {OpCodeId::TRAP, OpCode{0x7692, 0}},  {OpCodeId::SWM, OpCode{0x7693, 0}},
    {OpCodeId::JMP, OpCode{0x7694, 0}}};

// This table has a value to represent which arguments are to be dereferenced.
// A value of 5('b101) means Op0 and Op2 are to be dereferenced. For Op1 we'll directly use the value in the register
const std::unordered_map<OpCodeId, uint8_t> opCodeDereferenceMap = {
    {OpCodeId::ADD, 0b000},   {OpCodeId::SUB, 0b000},   {OpCodeId::MUL, 0b000},   {OpCodeId::DIV, 0b000},
    {OpCodeId::AND, 0b000},   {OpCodeId::OR, 0b000},    {OpCodeId::XOR, 0b000},   {OpCodeId::JZ, 0b00},
    {OpCodeId::JNZ, 0b00},    {OpCodeId::MOV, 0b00},    {OpCodeId::TSTB, 0b00},   {OpCodeId::SETZ, 0b0},
    {OpCodeId::SETO, 0b0},    {OpCodeId::SET, 0b0},     {OpCodeId::PUSH, 0b0},    {OpCodeId::POP, 0b0},
    {OpCodeId::NOT, 0b0},     {OpCodeId::SHFR, 0b0},    {OpCodeId::SHFL, 0b0},    {OpCodeId::INC, 0b0},
    {OpCodeId::DEC, 0b0},     {OpCodeId::NOP, 0b0},     {OpCodeId::STOP, 0b0},    {OpCodeId::ADD_RM, 0b01},
    {OpCodeId::ADD_MR, 0b10}, {OpCodeId::ADD_MM, 0b11}, {OpCodeId::SUB_RM, 0b01}, {OpCodeId::SUB_MR, 0b10},
    {OpCodeId::SUB_MM, 0b11}, {OpCodeId::MUL_RM, 0b01}, {OpCodeId::MUL_MR, 0b10}, {OpCodeId::MUL_MM, 0b11},
    {OpCodeId::DIV_RM, 0b01}, {OpCodeId::DIV_MR, 0b10}, {OpCodeId::DIV_MM, 0b11}, {OpCodeId::AND_RM, 0b01},
    {OpCodeId::AND_MR, 0b10}, {OpCodeId::AND_MM, 0b11}, {OpCodeId::OR_RM, 0b01},  {OpCodeId::OR_MR, 0b10},
    {OpCodeId::OR_MM, 0b11},  {OpCodeId::XOR_RM, 0b01}, {OpCodeId::XOR_MR, 0b10}, {OpCodeId::XOR_MM, 0b11},
    {OpCodeId::JZ_RM, 0b01},  {OpCodeId::JZ_MR, 0b10},  {OpCodeId::JZ_MM, 0b11},  {OpCodeId::JNZ_RM, 0b01},
    {OpCodeId::JNZ_MR, 0b10}, {OpCodeId::JNZ_MM, 0b11}, {OpCodeId::MOV_RM, 0b01}, {OpCodeId::MOV_MR, 0b10},
    {OpCodeId::MOV_MM, 0b11}, {OpCodeId::TSTB_M, 0b01}, {OpCodeId::SETZ_M, 0b1},  {OpCodeId::SETO_M, 0b1},
    {OpCodeId::SET_M, 0b1},   {OpCodeId::PUSH_M, 0b1},  {OpCodeId::POP_M, 0b1},   {OpCodeId::NOT_M, 0b1},
    {OpCodeId::SHFR_M, 0b1},  {OpCodeId::SHFL_M, 0b1},  {OpCodeId::INC_M, 0b1},   {OpCodeId::DEC_M, 0b1},
    {OpCodeId::STOP, 0b0},    {OpCodeId::JMP, 0b0}};

const std::unordered_map<std::string, OpCodeId> mnemonicTable = {
    {"ADD", OpCodeId::ADD},       {"SUB", OpCodeId::SUB},       {"MUL", OpCodeId::MUL},
    {"DIV", OpCodeId::DIV},       {"AND", OpCodeId::AND},       {"OR", OpCodeId::OR},
    {"XOR", OpCodeId::XOR},       {"JZ", OpCodeId::JZ},         {"JNZ", OpCodeId::JNZ},
    {"MOV", OpCodeId::MOV},       {"TSTB", OpCodeId::TSTB},     {"SETZ", OpCodeId::SETZ},
    {"SETO", OpCodeId::SETO},     {"SET", OpCodeId::SET},       {"PUSH", OpCodeId::PUSH},
    {"POP", OpCodeId::POP},       {"NOT", OpCodeId::NOT},       {"SHFR", OpCodeId::SHFR},
    {"SHFL", OpCodeId::SHFL},     {"INC", OpCodeId::INC},       {"DEC", OpCodeId::DEC},
    {"NOP", OpCodeId::NOP},       {"STOP", OpCodeId::STOP},     {"ADD_RM", OpCodeId::ADD_RM},
    {"ADD_MR", OpCodeId::ADD_MR}, {"ADD_MM", OpCodeId::ADD_MM}, {"SUB_RM", OpCodeId::SUB_RM},
    {"SUB_MR", OpCodeId::SUB_MR}, {"SUB_MM", OpCodeId::SUB_MM}, {"MUL_RM", OpCodeId::MUL_RM},
    {"MUL_MR", OpCodeId::MUL_MR}, {"MUL_MM", OpCodeId::MUL_MM}, {"DIV_RM", OpCodeId::DIV_RM},
    {"DIV_MR", OpCodeId::DIV_MR}, {"DIV_MM", OpCodeId::DIV_MM}, {"AND_RM", OpCodeId::AND_RM},
    {"AND_MR", OpCodeId::AND_MR}, {"AND_MM", OpCodeId::AND_MM}, {"OR_RM", OpCodeId::OR_RM},
    {"OR_MR", OpCodeId::OR_MR},   {"OR_MM", OpCodeId::OR_MM},   {"XOR_RM", OpCodeId::XOR_RM},
    {"XOR_MR", OpCodeId::XOR_MR}, {"XOR_MM", OpCodeId::XOR_MM}, {"JZ_RM", OpCodeId::JZ_RM},
    {"JZ_MR", OpCodeId::JZ_MR},   {"JZ_MM", OpCodeId::JZ_MM},   {"JNZ_RM", OpCodeId::JNZ_RM},
    {"JNZ_MR", OpCodeId::JNZ_MR}, {"JNZ_MM", OpCodeId::JNZ_MM}, {"MOV_RM", OpCodeId::MOV_RM},
    {"MOV_MR", OpCodeId::MOV_MR}, {"MOV_MM", OpCodeId::MOV_MM}, {"TSTB_M", OpCodeId::TSTB_M},
    {"SETZ_M", OpCodeId::SETZ_M}, {"SETO_M", OpCodeId::SETO_M}, {"SET_M", OpCodeId::SET_M},
    {"PUSH_M", OpCodeId::PUSH_M}, {"POP_M", OpCodeId::POP_M},   {"NOT_M", OpCodeId::NOT_M},
    {"SHFR_M", OpCodeId::SHFR_M}, {"SHFL_M", OpCodeId::SHFL_M}, {"INC_M", OpCodeId::INC_M},
    {"DEC_M", OpCodeId::DEC_M},   {"TRAP", OpCodeId::TRAP},     {"SWM", OpCodeId::SWM},
    {"JMP", OpCodeId::JMP}};

const std::unordered_map<std::string, RegisterId> registerMap = {
    {"RAC", RegisterId::RAC}, {"RFL", RegisterId::RFL}, {"RIP", RegisterId::RIP}, {"RSP", RegisterId::RSP},
    {"RBP", RegisterId::RBP}, {"R0", RegisterId::R0},   {"R1", RegisterId::R1},   {"R2", RegisterId::R2},
    {"R3", RegisterId::R3},   {"R4", RegisterId::R4},   {"R5", RegisterId::R5},   {"R6", RegisterId::R6},
    {"R7", RegisterId::R7},   {"R8", RegisterId::R8},   {"R9", RegisterId::R9},   {"R10", RegisterId::R10},
    {"R11", RegisterId::R11}};

const std::unordered_map<uint16_t, OpCodeId> opCodeValuesTable = {
    {0x0, OpCodeId::ADD},      {0x1, OpCodeId::SUB},      {0x2, OpCodeId::MUL},      {0x3, OpCodeId::DIV},
    {0x4, OpCodeId::AND},      {0x5, OpCodeId::OR},       {0x6, OpCodeId::XOR},      {0x70, OpCodeId::JZ},
    {0x71, OpCodeId::JNZ},     {0x72, OpCodeId::MOV},     {0x75, OpCodeId::TSTB},    {0x760, OpCodeId::SETZ},
    {0x761, OpCodeId::SETO},   {0x762, OpCodeId::SET},    {0x763, OpCodeId::PUSH},   {0x764, OpCodeId::POP},
    {0x765, OpCodeId::NOT},    {0x766, OpCodeId::SHFR},   {0x767, OpCodeId::SHFL},   {0x768, OpCodeId::INC},
    {0x963, OpCodeId::DEC},    {0x7690, OpCodeId::NOP},   {0x7691, OpCodeId::STOP},  {0x77, OpCodeId::ADD_RM},
    {0x78, OpCodeId::ADD_MR},  {0x79, OpCodeId::ADD_MM},  {0x7a, OpCodeId::SUB_RM},  {0x7b, OpCodeId::SUB_MR},
    {0x7c, OpCodeId::SUB_MM},  {0x7d, OpCodeId::MUL_RM},  {0x7e, OpCodeId::MUL_MR},  {0x7f, OpCodeId::MUL_MM},
    {0x80, OpCodeId::DIV_RM},  {0x81, OpCodeId::DIV_MR},  {0x82, OpCodeId::DIV_MM},  {0x83, OpCodeId::AND_RM},
    {0x84, OpCodeId::AND_MR},  {0x85, OpCodeId::AND_MM},  {0x86, OpCodeId::OR_RM},   {0x87, OpCodeId::OR_MR},
    {0x88, OpCodeId::OR_MM},   {0x89, OpCodeId::XOR_RM},  {0x8a, OpCodeId::XOR_MR},  {0x8b, OpCodeId::XOR_MM},
    {0x8c, OpCodeId::JZ_RM},   {0x8d, OpCodeId::JZ_MR},   {0x8e, OpCodeId::JZ_MM},   {0x8f, OpCodeId::JNZ_RM},
    {0x90, OpCodeId::JNZ_MR},  {0x91, OpCodeId::JNZ_MM},  {0x92, OpCodeId::MOV_RM},  {0x93, OpCodeId::MOV_MR},
    {0x94, OpCodeId::MOV_MM},  {0x95, OpCodeId::TSTB_M},  {0x76a, OpCodeId::SETZ_M}, {0x76b, OpCodeId::SETO_M},
    {0x76c, OpCodeId::SET_M},  {0x76d, OpCodeId::PUSH_M}, {0x76e, OpCodeId::POP_M},  {0x76f, OpCodeId::NOT_M},
    {0x960, OpCodeId::SHFR_M}, {0x961, OpCodeId::SHFL_M}, {0x962, OpCodeId::INC_M},  {0x964, OpCodeId::DEC_M},
    {0x7692, OpCodeId::TRAP},  {0x7693, OpCodeId::SWM},   {0x7694, OpCodeId::JMP}};
