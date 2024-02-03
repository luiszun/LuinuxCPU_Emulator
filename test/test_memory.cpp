#include <gtest/gtest.h>

#include "common.h"
#include "memory.h"

class TestNVMemory : public NVMemory<uint16_t>
{
  public:
    TestNVMemory(size_t size, std::string filename) : NVMemory<uint16_t>(size, filename)
    {
    }

    void FlushOut()
    {
        _FlushOut();
    }
    void FlushIn()
    {
        _FlushIn();
    }
};

using Memory16 = Memory<uint16_t>;

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
    TestNVMemory mem(0x10000, "test_nvmemory.bin");
    std::array<uint8_t, 17> memdmp = {0xde, 0xad, 0xbe, 0xef, 0x0,  0x0,  0x0,  0x0, 0xc0,
                                      0x8b, 0xad, 0xf0, 0x0d, 0x00, 0x00, 0x00, 0x00};
    // Last 4 bytes are different
    std::array<uint8_t, 17> memdmp2 = {0xde, 0xad, 0xbe, 0xef, 0x0,  0x0,  0x0,  0x0, 0xc0,
                                       0x8b, 0xad, 0xf0, 0x0d, 0x01, 0x02, 0x03, 0x04};

    assert(memdmp.size() == memdmp2.size());

    // First read and make sure it was done correctly the last time
    for (unsigned i = 0; i < memdmp.size(); ++i)
    {
        ASSERT_EQ(mem.Read8(i), memdmp[i]);
    }

    // Now write
    for (unsigned i = 0; i < memdmp.size(); ++i)
    {
        mem.Write8(i, memdmp2[i]);
    }

    mem.FlushOut();

    // Just to make sure we did read something different
    for (unsigned i = 0; i < memdmp.size(); ++i)
    {
        mem.Write8(i, 0xff);
    }

    mem.FlushIn();

    // Now check we got what we had written to disk
    for (unsigned i = 0; i < memdmp.size(); ++i)
    {
        ASSERT_EQ(mem.Read8(i), memdmp2[i]);
    }

    // Great we know it worked. Let's take the file back to normal.
    for (unsigned i = 0; i < memdmp.size(); ++i)
    {
        mem.Write8(i, memdmp[i]);
    }

    mem.FlushOut();

    // Now to make sure we left the file as we found it.
    mem.FlushIn();
    for (unsigned i = 0; i < memdmp.size(); ++i)
    {
        ASSERT_EQ(mem.Read8(i), memdmp[i]);
    }
}

TEST(TestMemorySuite, TestWriteShellcode)
{
    Memory16 mem(0xff);

    const char shellCode[] = "\xde\xad\xbe\xef";
    mem.WritePayload(1, shellCode, 4);

    ASSERT_EQ(mem.Read8(0), 0);
    ASSERT_EQ(mem.Read8(1), 0xde);
    ASSERT_EQ(mem.Read8(2), 0xad);
    ASSERT_EQ(mem.Read8(3), 0xbe);
    ASSERT_EQ(mem.Read8(4), 0xef);

    ASSERT_EQ(mem.Read16(0), 0x00de);
    ASSERT_EQ(mem.Read16(2), 0xadbe);
    ASSERT_EQ(mem.Read16(4), 0xef00);
}