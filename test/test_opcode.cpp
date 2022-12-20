#include <algorithm>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>

#include <opcode.h>

TEST(TestOpCodes_Overlapping, BasicAssertions)
{
    std::array<uint8_t, 0x10000> usedOpcode;
    usedOpcode.fill(0);

    for (auto i = opCodeTable.begin(); i != opCodeTable.end(); ++i)
    {
        const auto opName = i->first;
        const auto opCodeStruct = i->second;
        const auto nOperands = opCodeStruct.argCount;
        const auto opCode = opCodeStruct.opCode;

        auto opCodeStart = opCode << (nOperands * 4);

        const auto nextOpCode = opCodeStart + (1 << (nOperands * 4));
        std::cout << std::hex << opName << " 0x" << std::setfill('0') << std::setw(4) << opCodeStart << " 0x"
                  << std::setfill('0') << std::setw(4) << nextOpCode - 1 << std::endl;
        ASSERT_TRUE(opCodeStart < 0x10000);
        for (auto i = opCodeStart; i < nextOpCode; ++i)
        {
            ASSERT_EQ(usedOpcode[i], 0);
            usedOpcode[i] = 1;
        }
    }

    // Indates when we got the beginning of the free opcodes.
    // Now we need to find the end
    bool gotBeginning = false;
    for (auto i = 0; i < 0xffff; ++i)
    {
        if (!gotBeginning)
        {
            if (usedOpcode[i] != 0)
                continue;
            gotBeginning = true;
            std::cout << "Free opcode from 0x" << std::hex << std::setfill('0') << std::setw(4) << i;
        }
        else
        {
            if (usedOpcode[i] == 0)
                continue;
            gotBeginning = false;
            std::cout << " to 0x" << std::hex << std::setfill('0') << std::setw(4) << i - 1 << std::endl;
        }
    }

    // Did we finish with an open range?
    if (gotBeginning)
    {
        std::cout << " to 0xffff" << std::endl;
    }
}
