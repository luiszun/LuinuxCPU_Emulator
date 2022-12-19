#include "opcode.h"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <iomanip>
// TODO: Add real test fw :)

bool TestOpcodeOverlap()
{
    std::array<uint8_t, 0xffff> usedOpcode;
    usedOpcode.fill(0);

    for (auto &opCode : opCodeTable)
    {
        const auto opName = std::get<0>(opCode);
        const auto nOperands = std::get<2>(opCode);
        auto opCodeStart = std::get<1>(opCode) << (nOperands * 4);

        const auto nextOpCode = opCodeStart + (1 << (nOperands * 4));
        std::cout << std::hex << opName << " 0x" << std::setfill('0') << std::setw(4) << opCodeStart << " 0x" << std::setfill('0') << std::setw(4) << nextOpCode << std::endl;
        for (auto i = opCodeStart; i < nextOpCode; ++i)
        {
            assert(usedOpcode[i] == 0);
            usedOpcode[i] = 1;
        }
    }
    return true;
}

int main()
{
    unsigned failed = 0;
    if (!TestOpcodeOverlap())
    {
        failed++;
        std::cout << "TestOpcodeOverlap failed" << std::endl;
    }
}