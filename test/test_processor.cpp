#include <gtest/gtest.h>

#include "common.h"
#include "processor.h"

TEST(TestOpCodesSuite, TestProcessorExistence)
{
    Processor cpu(std::string("build/test/test_nvmemory.bin"));
}