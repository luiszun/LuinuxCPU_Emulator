#include <iostream>
#include <algorithm>
#include <iomanip>
#include <gtest/gtest.h>

#include <opcode.h>

TEST(TestOpCodes_Overlapping, BasicAssertions)
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
            ASSERT_EQ(usedOpcode[i], 0);
            usedOpcode[i] = 1;
        }
    }
}
