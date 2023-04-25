#include <gtest/gtest.h>

#include "common.h"
#include "memory.h"
#include "processor.h"

using Memory16 = Memory<uint16_t>;

// Written in little endian
// unsigned char loop10xShellCode[] = "\x25\x76\x00\x10\x26\x76\x01\x00\x67\x45\x25\x76\x0e\x00";

unsigned char loop10xShellCode[] = "\x76\x25\x00\x10\x76\x26\x00\x01\x45\x67\x76\x25\x00\x0e";

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

    OpCodeId GetDecodedOP()
    {
        return _decodedOpCodeId;
    }

    RegisterId GetArgs(unsigned argN)
    {
        return _instructionArgs[argN];
    }
};

TEST(TestProcessorSuite, TestProcessorExistence)
{
    Memory16 programMemory(0x10000);
    Processor cpu(programMemory);
}

TEST(TestProcessorSuite, TestFetch)
{
    Memory16 programMemory(0x10000);
    programMemory.Write(0, loop10xShellCode, 14);
    TestProcessor cpu(programMemory);
    cpu.FetchInstruction();
    ASSERT_EQ(cpu.GetFetchedInstruction(), 0x7625);
    cpu.FetchInstruction();
    ASSERT_EQ(cpu.GetFetchedInstruction(), 0x0010);
}

TEST(TestProcessorSuite, TestFetchAndDecode)
{
    Memory16 programMemory(0x10000);
    programMemory.Write(0, loop10xShellCode, 14);
    TestProcessor cpu(programMemory);
    cpu.FetchInstruction();
    cpu.DecodeInstruction();
    ASSERT_EQ(cpu.GetDecodedOP(), OpCodeId::SET);
    ASSERT_EQ(cpu.GetArgs(0), RegisterId::R0);

    // Need to do this to finish the cycle before starting another
    cpu.ExecuteInstruction();
    cpu.FetchInstruction();
    cpu.DecodeInstruction();
    ASSERT_EQ(cpu.GetDecodedOP(), OpCodeId::SET);
    ASSERT_EQ(cpu.GetArgs(0), RegisterId::R1);
}