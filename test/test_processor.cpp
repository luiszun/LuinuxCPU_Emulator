#include <gtest/gtest.h>

#include "assembler.h"
#include "common.h"
#include "memory.h"
#include "processor.h"

using Memory16 = Memory<uint16_t>;

const char loop10xShellCode[] = "\x76\x25\x00\x10\x76\x26\x00\x01\x45\x67\x76\x25\x00\x0e";

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

    std::unordered_map<RegisterId, Register> &GetRegisters()
    {
        return _registers;
    }

    Memory16 &GetMainMemory()
    {
        return _mainMemory;
    }

    uint16_t DereferenceRegisterRead(RegisterId reg)
    {
        return _DereferenceRegisterRead(reg);
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
    programMemory.WritePayload(0, loop10xShellCode, 14);
    TestProcessor cpu(programMemory);
    cpu.FetchInstruction();
    ASSERT_EQ(cpu.GetFetchedInstruction(), 0x7625);
    cpu.FetchInstruction();
    ASSERT_EQ(cpu.GetFetchedInstruction(), 0x0010);
}

TEST(TestProcessorSuite, TestFetchAndDecode)
{
    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, loop10xShellCode, 14);
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

TEST(TestProcessorSuite, TestRegisterDereference)
{
    Memory16 programMemory(0x10000);
    TestProcessor cpu(programMemory);
    auto &rac = cpu.GetRegisters().at(RegisterId::RAC);
    rac.Write(0xdead);

    // write 0xbeef at the address 0xdead
    cpu.GetMainMemory().Write8(0xdead, 0xbe);
    cpu.GetMainMemory().Write8(0xdead + 1, 0xef);

    uint16_t derefVal = cpu.DereferenceRegisterRead(RegisterId::RAC);
    ASSERT_EQ(derefVal, 0xbeef);
}

/*
Instructions that need testing:
ADD
MUL
DIV
AND
OR
XOR
JZ
MOV
LOAD
STOR
TSTB
SETZ
SETO
PUSH
POP
NOT
SHFR
SHFL
DEC
NOP
ADD_RM
ADD_MR
ADD_MM
SUB_RM
SUB_MR
SUB_MM
MUL_RM
MUL_MR
MUL_MM
DIV_RM
DIV_MR
DIV_MM
AND_RM
AND_MR
AND_MM
OR_RM
OR_MR
OR_MM
XOR_RM
XOR_MR
XOR_MM
JZ_RM
JZ_MR
JZ_MM
JNZ_RM
JNZ_MR
JNZ_MM
MOV_RM
MOV_MR
MOV_MM
TSTB_M
SETZ_M
SETO_M
SET_M
PUSH_M
POP_M
NOT_M
SHFR_M
SHFL_M
DEC_M
*/
TEST(TestProcessorPrograms, Test10xLoop)
{
    Assembler asmObj;
    std::string program = "SET R0, 10 ; This is the number of times it will loop\n"
                          "SET R10, 0 ; Initialize R10 to be our counter\n"
                          "goto:R2 ; loop on R2\n"
                          "INC R10\n"
                          "SUB R0, R10, R1\n"
                          "JNZ R1, R2\n"
                          "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.GetRegisters().at(RegisterId::R0).Read(), 10);
    ASSERT_EQ(cpu.GetRegisters().at(RegisterId::R10).Read(), 10);
}

TEST(TestProcessorPrograms, Test10xLoop_RM)
{
    Assembler asmObj;
    std::string program = "SET R0, 10 ; This is the number of times it will loop\n"
                          "SET R10, h'100 ; 0x100 will contain the counter. R10 is the ptr\n"
                          "goto:R2 ; loop on R2\n"
                          "INC_M R10\n"
                          "SUB_RM R0, R10\n"
                          "JNZ RAC, R2\n"
                          "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.GetRegisters().at(RegisterId::R0).Read(), 10);
    ASSERT_EQ(cpu.GetRegisters().at(RegisterId::R10).Read(), 0x100);
    ASSERT_EQ(cpu.DereferenceRegisterRead(RegisterId::R10), 10);
}

TEST(TestProcessorPrograms, TestAluOps)
{
    Assembler asmObj;
    // 2 + 3
    // 5 * 3
    // 15 - 6
    // 9 / 3
    std::string program = "SET R0, 2\n"
                          "SET R1, 3\n"
                          "SET R2, 5\n"
                          "SET R3, 6\n"
                          "ADD R0, R1, R10 ; 2+3\n"
                          "MUL R10, R1, R10 ; 5*3\n"
                          "SUB R10, R3, R10 ; 15-6\n"
                          "DIV R10, R1, R10 ; 9/3\n"
                          "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.GetRegisters().at(RegisterId::R10).Read(), 3);
}