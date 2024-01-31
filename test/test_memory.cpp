#include <gtest/gtest.h>

#include "common.h"
#include "memory.h"

using Memory16 = Memory<uint16_t>;
using NVMemory16 = NVMemory<uint16_t>;

TEST(TestMemorySuite, TestBasicReadWrite)
{
    Memory16 mem(0xff);
    mem.Write8(0, 0xde);
    auto word = mem.Read8(0);
    EXPECT_EQ(word, 0xde);

    EXPECT_ANY_THROW(mem.Write8(0xffaa, 0xad));
}


// TODO Test write
TEST(TestMemorySuite, TestNonVolatileMemoryReadWrite)
{
    NVMemory16 mem(0x10000, "test_nvmemory.bin");
    std::array<uint8_t, 17>memdmp = {0xde, 0xad, 0xbe, 0xef, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x8b, 0xad, 0xf0, 0x0d, 0x00, 0x00, 0x00, 0x00};
    // Last 4 bytes are different
    std::array<uint8_t, 17>memdmp2 = {0xde, 0xad, 0xbe, 0xef, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x8b, 0xad, 0xf0, 0x0d, 0x01, 0x02, 0x03, 0x04};

    assert(memdmp.size() == memdmp2.size());

    for (unsigned i = 0; i < memdmp.size(); ++i)
    {
        // First read and make sure it was done correctly the last time
        ASSERT_EQ(mem.Read8(i), memdmp[i]);
    }
}

TEST(TestMemorySuite, TestWriteShellcode)
{
    Memory16 mem(0xff);

    const unsigned char shellCode[] = "\xde\xad\xbe\xef";
    mem.Write(1, shellCode, 4);

    ASSERT_EQ(mem.Read8(0), 0);
    ASSERT_EQ(mem.Read8(1), 0xde);
    ASSERT_EQ(mem.Read8(2), 0xad);
    ASSERT_EQ(mem.Read8(3), 0xbe);
    ASSERT_EQ(mem.Read8(4), 0xef);
}