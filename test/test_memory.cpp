#include <gtest/gtest.h>

#include "common.h"
#include "memory.h"

using Memory16 = Memory<uint16_t>;
using NVMemory16 = NVMemory<uint16_t>;

TEST(TestMemorySuite, TestBasicReadWrite)
{
    Memory16 mem(0xff);
    mem.Write(0, 0xde);
    auto word = mem.Read(0);
    EXPECT_EQ(word, 0xde);

    EXPECT_ANY_THROW(mem.Write(0xffaa, 0xad));
}

TEST(TestMemorySuite, TestNonVolatileMemory)
{
    NVMemory16 mem(0x10000, "test_nvmemory.bin");
    uint8_t memdmp[] = {0xde, 0xad, 0xbe, 0xef, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x8b, 0xad, 0xf0, 0x0d};

    for (unsigned i = 0; i < 13; ++i)
    {
        // First read and make sure it was done correctly the last time
        ASSERT_EQ(mem.Read(i), memdmp[i]);

        // Write back
        mem.Write(i, memdmp[i]);
    }
}

TEST(TestMemorySuite, TestWriteShellcode)
{
    Memory16 mem(0xff);

    mem.Write(1, "\xde\xad\xbe\xef");

    ASSERT_EQ(mem.Read(0), 0);
    ASSERT_EQ(mem.Read(1), 0xde);
    ASSERT_EQ(mem.Read(2), 0xad);
    ASSERT_EQ(mem.Read(3), 0xbe);
    ASSERT_EQ(mem.Read(4), 0xef);
}