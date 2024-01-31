#include <gtest/gtest.h>

#include "common.h"
#include "register.h"

using Memory8 = Memory<uint8_t>;

TEST(TestRegisterSuite, TestBasicReadWrite)
{
    Memory8 mem(0xff);

    Register R0(static_cast<uint8_t>(RegisterId::R0) * uint8_t{2}, mem);
    Register RAC(static_cast<uint8_t>(RegisterId::RAC) * uint8_t{2}, mem);

    R0.Write(0xdead);
    RAC.Write(0xbeef);
    auto r0Val = R0.Read();
    auto racVal = RAC.Read();

    EXPECT_EQ(r0Val, 0xdead);
    EXPECT_EQ(racVal, 0xbeef);

    racVal = mem.Read8(0x0);
    EXPECT_EQ(racVal, 0xbe);
    racVal = mem.Read8(0x1);
    EXPECT_EQ(racVal, 0xef);
}