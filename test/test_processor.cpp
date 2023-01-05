#include <gtest/gtest.h>

#include "common.h"
#include "memory.h"
#include "processor.h"

using Memory16 = Memory<uint16_t>;

class TestProcessor : public Processor
{
  public:
    TestProcessor(Memory16 &mem) : Processor(mem)
    {
    }

    void FetchInstruction()
    {
        _FetchInstruction();
    }
    void DecodeInstruction()
    {
        _DecodeInstruction();
    }
    void ExecuteInstruction()
    {
        _ExecuteInstruction();
    }

    uint16_t GetFetchedInstruction()
    {
        return _fetchedInstruction;
    }
};

TEST(TestProcessorSuite, TestProcessorExistence)
{
    Memory16 programMemory(0x10000);
    Processor cpu(programMemory);
}

TEST(TestProcessorSuite, TestFetchAndDecode)
{
    unsigned char loop10xShellCode[] = "\x25\x76\x00\x10\x26\x76\x01\x00\x67\x45\x25\x76\x0e\x00";
    Memory16 programMemory(0x10000);
    programMemory.Write(0, loop10xShellCode, 14);
    TestProcessor cpu(programMemory);
    cpu.FetchInstruction();
    ASSERT_EQ(cpu.GetFetchedInstruction(), 0x2576);
    cpu.FetchInstruction();
    ASSERT_EQ(cpu.GetFetchedInstruction(), 0x0010);
}