#include <array>
#include <string>
#include <tuple>

// Mnemonic, Opcode, nArgumens
std::tuple<std::string, uint8_t, uint8_t> opCodeTable[] = {
    {"ADD", 0x0, 3}, {"SUB", 0x1, 3}, {"MUL", 0x2, 3}};
