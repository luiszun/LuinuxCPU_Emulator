#include <gtest/gtest.h>

#include "common.h"
#include "memory.h"

using Memory16 = Memory<uint16_t>;

TEST(TestOpCodes_BasicMemory, BasicAssertions)
{
    Memory16 mem(0xff);
    mem.Write(0, 0xde);
    auto word = mem.Read(0);
    EXPECT_EQ(word, 0xde);

    EXPECT_ANY_THROW(mem.Write(0xffaa, 0xad));
}