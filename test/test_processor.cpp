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

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R0), 10);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R10), 10);
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

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R0), 10);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R10), 0x100);
    ASSERT_EQ(cpu.DereferenceRegisterRead(RegisterId::R10), 10);
}

TEST(TestProcessorPrograms, Test10xLoopDec)
{
    Assembler asmObj;
    std::string program = "SET R0, 10 ; This is the number of times it will loop\n"
                          "SET R1, 0; This will act as our counter to test\n"
                          "goto:R2 ; loop on R2\n"
                          "DEC R0\n"
                          "INC R1\n"
                          "JNZ R0, R2\n"
                          "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R1), 10);
}

TEST(TestProcessorPrograms, Test10xLoopDec_M)
{
    Assembler asmObj;
    std::string program = "SET R0, h'200\n"
                          "SET_M R0 10\n"
                          "SET R1, 0; This will act as our counter to test\n"
                          "goto:R2 ; loop on R2\n"
                          "DEC_M R0\n"
                          "INC R1\n"
                          "JNZ_MR R0, R2\n"
                          "MOV R1, R1\n"
                          "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R1), 10);
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

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R10), 3);
}

TEST(TestProcessorPrograms, TestAluOps_MM)
{
    Assembler asmObj;
    // 2 + 3
    // 5 * 3
    // 15 - 6
    // 9 / 3
    std::string program = "SET R0, 0; *R0 = 2\n"
                          "SET_M R0, 2\n"
                          "SET R1, 2; *R1 = 3\n"
                          "SET_M R1, 3\n"
                          "SET R2, 4; *R2 = 5\n"
                          "SET_M R2, 5\n"
                          "SET R3, 6; *R3 = 6\n"
                          "SET_M R3, 6\n"
                          "SET R10, h'100; *R10 contains partial results\n"
                          "ADD_MM R0, R1; 2+3\n"
                          "MOV_RM RAC, R10\n"
                          "MUL_MM R10, R1; 5*3\n"
                          "MOV_RM RAC, R10\n"
                          "SUB_MM R10, R3; 15-6\n"
                          "MOV_RM RAC, R10\n"
                          "DIV_MM R10, R1; 9/3\n"
                          "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 3);
}

TEST(TestProcessorPrograms, TestAluOps_MR)
{
    Assembler asmObj;
    // 2 + 3
    // 5 * 3
    // 15 - 6
    // 9 / 3
    std::string program = "SET R0, h'100\n"
                          "SET_M R0, 2\n"
                          "SET R1, 3\n"
                          "SET R2, 5\n"
                          "SET R3, 6\n"
                          "ADD_MR R0, R1; 2+3\n"
                          "MOV_RM RAC, R10\n"
                          "MUL_MR R10, R1; 5*3\n"
                          "MOV_RM RAC, R10\n"
                          "SUB_MR R10, R3; 15-6\n"
                          "MOV_RM RAC, R10\n"
                          "DIV_MR R10, R1; 9/3\n"
                          "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 3);
}

TEST(TestProcessorPrograms, TestAluOps_RM)
{
    Assembler asmObj;
    // 2 + 3
    // 5 * 3
    // 15 - 6
    // 9 / 3
    std::string program = "SET R0, 2\n"
                          "SET R7, h'100 ; this ptr will hold our right operands\n"
                          "SET_M R7, 3\n"
                          "ADD_RM R0, R7; 2+3\n"
                          "MUL_RM RAC, R7; 5*3\n"
                          "SET_M R7, 6\n"
                          "SUB_RM RAC, R7; 15-6\n"
                          "SET_M R7, 3\n"
                          "DIV_RM RAC, R7; 9/3\n"
                          "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 3);
}

TEST(TestProcessorPrograms, TestBitOps)
{
    Assembler asmObj;
    std::string program = "SET R0, 1\n"
                          "SETO R1 ; h'ffff\n"
                          "SHFL R0 ; h'2\n"
                          "SHFR R1 ; h'7fff\n"
                          "XOR R0, R1, R1 ; h'7ffd\n"
                          "SET R2 h'8000\n"
                          "OR R2, R0, R0 ; h'8002\n"
                          "SET R3 h'7fff\n"
                          "AND R3, R0, R4 ; h'2\n"
                          "NOT R4 ; h'fffd\n"
                          "SET R5 0\n"
                          "TSTB R5, R4 ; FLR:Zero got 1\n"
                          "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R0), 0x8002);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R1), 0x7ffd);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R2), 0x8000);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R4), 0xfffd);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::RFL) & static_cast<uint16_t>(FlagsRegister::Zero), 1);
}

TEST(TestProcessorPrograms, TestBitOps_Indirect)
{
    Assembler asmObj;
    std::string program = "SET R0, 0\n"
                          "SET R1, 2\n"
                          "SET_M R0 h'aaaa\n"
                          "SET_M R1 h'5555\n"
                          "OR_MM R0 R1 ; RAC == 0xffff\n"
                          "XOR_RM RAC R0 ; RAC == h'5555\n"
                          "TRAP ; TEST VAL\n"
                          "OR_RM RAC R0 ; RAC == 0xffff\n"
                          "SHFR_M R0 ; R0==0x5555\n"
                          "SHFL_M R1 ; R1==0xaaaa\n"
                          "TRAP ; TEST VAL\n"
                          "TSTB_M R0, R0 ; RFL Zero=1\n"
                          "XOR_MR R0, RAC ; ffff^5555 RAC == 0xaaaa\n"
                          "OR_RM RAC, R0 ; RAC=0xffff \n"
                          "AND_RM RAC, R1; RAC==0xaaaa \n"
                          "TRAP ; TEST VAL\n"
                          "; RAC=aaaa R0=5555 R1=aaaa\n"
                          "AND_MR R0, RAC ; 0x5555 & 0xaaaa -> RAC:0\n"
                          "OR_MR R0, RAC ; 5555 | 0 -> RAC:0x5555\n"
                          "MOV RAC, R10 ; R10 = 0x5555\n"
                          "XOR_MM R0, R1\n"
                          "MOV RAC, R9 ; R9 = 0xffff\n"
                          "AND_MM R0, R1 ; RAC = 0\n"
                          "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    // TRAP 1
    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 0x5555);
    cpu.WriteRegister(RegisterId::RFL, 0);
    cpu.ExecuteAll();

    // TRAP 2
    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 0xffff);
    ASSERT_EQ(cpu.DereferenceRegisterRead(RegisterId::R0), 0x5555);
    ASSERT_EQ(cpu.DereferenceRegisterRead(RegisterId::R1), 0xaaaa);
    cpu.WriteRegister(RegisterId::RFL, 0);
    cpu.ExecuteAll();

    // TRAP 3
    FlagsObject f(cpu.ReadRegister(RegisterId::RFL));
    ASSERT_EQ(f.flags.Zero, 1);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 0xaaaa);
    cpu.WriteRegister(RegisterId::RFL, 0);
    cpu.ExecuteAll();    

    // TRAP 4
    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 0);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R10), 0x5555);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R9), 0xffff);
}

/*
Instructions that need testing:


JZ
MOV
LOAD
STOR
SETZ
PUSH
POP
NOT
NOP
JZ_RM
JZ_MM
JNZ_RM
JNZ_MM
MOV_MM
MOV_MR
SETZ_M
SETO_M
PUSH_M
POP_M
NOT_M

*/