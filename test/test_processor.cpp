#include <gtest/gtest.h>

#include "common.h"
#include "memory.h"
#include "processor.h"

using Memory16 = Memory<uint16_t>;
TEST(TestOpCodesSuite, TestProcessorExistence)
{
    Memory16 programMemory(0x10000);
    Processor cpu(programMemory);
}