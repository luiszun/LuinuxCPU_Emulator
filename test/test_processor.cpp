#include <gtest/gtest.h>

#include "common.h"
#include "memory.h"
#include "processor.h"

using Memory16 = Memory<uint16_t>;
TEST(TestProcessorSuite, TestProcessorExistence)
{
    Memory16 programMemory(0x10000);
    Processor cpu(programMemory);
}

TEST(TestProcessorSuite, TestFetchAndDecode)
{
    std::string loop10xShellCode = "\x25\x76\x00\x01\x26\x76\x01\x00\x67\x45\x25\x76\x0e\x00";
}