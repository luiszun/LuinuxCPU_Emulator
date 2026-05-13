#include <gtest/gtest.h>

#include "assembler.h"
#include "common.h"
#include "memory.h"
#include "processor.h"
#include "utils.h"

using Memory16 = Memory<uint16_t>;
using TestNVMemory = NVMemory<uint16_t>;

const char loop10xShellCode[] = "\x76\x25\x00\x10\x76\x26\x00\x01\x45\x67\x76\x25\x00\x0e";

class TestProcessor : public Processor
{
   public:
    TestProcessor(Memory16& mem, std::shared_ptr<TestNVMemory> nvram = nullptr)
        : Processor(mem, nvram)
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

    std::unordered_map<RegisterId, Register>& GetRegisters()
    {
        return _registers;
    }

    Memory16& GetMainMemory()
    {
        return *_mainMemory;
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
    auto& rac = cpu.GetRegisters().at(RegisterId::RAC);
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
    std::string program =
        "SET R0, 10 ; This is the number of times it will loop\n"
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

TEST(TestProcessorPrograms, Test10xLoopDec)
{
    Assembler asmObj;
    std::string program =
        "SET R0, 10 ; This is the number of times it will loop\n"
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

TEST(TestProcessorPrograms, TestAluOps)
{
    Assembler asmObj;
    // 2 + 3
    // 5 * 3
    // 15 - 6
    // 9 / 3
    std::string program =
        "SET R0, 2\n"
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

// Moved ALU flag tests to test/test_alu_flags.cpp
// Moved ALU flag tests to test/test_alu_flags.cpp
TEST(TestProcessorPrograms, TestBitOps)
{
    Assembler asmObj;
    std::string program =
        "SET R0, 1\n"
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

TEST(TestProcessorPrograms, TestSWM)
{
    Assembler asmObj;
    std::string program =
        "TRAP\n"
        "SWM\n"
        "TRAP\n"
        "SWM\n"
        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);

    std::shared_ptr<NVMemory16> nvram = std::make_shared<NVMemory16>(0x10000, "test_nvmemory.bin");
    TestProcessor cpu(programMemory, nvram);
    cpu.ExecuteAll();
    {
        // TRAP 1
        FlagsObject f(cpu.ReadRegister(RegisterId::RFL));
        ASSERT_EQ(f.flags.Memory, 0);
        cpu.WriteRegister(RegisterId::RFL, 0);
        cpu.ExecuteAll();
    }
    {
        // TRAP 2
        FlagsObject f(cpu.ReadRegister(RegisterId::RFL));
        ASSERT_EQ(f.flags.Memory, 1);
        f.flags.Trap = 0;
        cpu.WriteRegister(RegisterId::RFL, f.value);
        cpu.ExecuteAll();
    }
    {
        // STOP
        FlagsObject f(cpu.ReadRegister(RegisterId::RFL));
        ASSERT_EQ(f.flags.Memory, 0);
    }
}

TEST(TestProcessorPrograms, TestNVRamWriting)
{
    Assembler asmObj;
    std::shared_ptr<TestNVMemory> nvram =
        std::make_shared<TestNVMemory>(0x10000, "test_nvmemory.bin");

    uint16_t topAddress = 0x10000 - 2;

    while (0 != nvram->Read16(topAddress))
    {
        topAddress -= 2;
    }
    std::string program =
        "SWM\n"
        "SET R0, " +
        std::to_string(topAddress) +
        "\n"
        "TRAP\n"
        "SET R0, h'ffff\n"
        "TRAP\n"
        "SWM\n"
        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory, nvram);
    cpu.ExecuteAll();

    {
        // TRAP 1
        FlagsObject f(cpu.ReadRegister(RegisterId::RFL));
        f.flags.Trap = 0;
        cpu.WriteRegister(RegisterId::RFL, f.value);
        ASSERT_EQ(cpu.DereferenceRegisterRead(RegisterId::R0), 0);
        cpu.ExecuteAll();
    }
    {
        // TRAP 2
        FlagsObject f(cpu.ReadRegister(RegisterId::RFL));
        f.flags.Trap = 0;
        cpu.WriteRegister(RegisterId::RFL, f.value);
        ASSERT_EQ(cpu.DereferenceRegisterRead(RegisterId::R0), 0xffff);
        cpu.ExecuteAll();

        // Now using sram
        ASSERT_EQ(cpu.DereferenceRegisterRead(RegisterId::R0), 0);
    }
}

TEST(TestProcessorPrograms, TestNonNVRamFail)
{
    Assembler asmObj;
    std::string program =
        "SWM\n"
        "STOP\n";
    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    EXPECT_ANY_THROW(cpu.ExecuteAll());
}

TEST(TestProcessorPrograms, TestJE)
{
    Assembler asmObj;
    std::string program =
        "; This program force tests JE and JNE by following "
        "the propoer jumps and ensuring the comply\n"
        "; RAC(dead)== R2(dead) RAC->cafe\n"
        "; RAC(dead)!= R2(dead) RAC->f00d\n"
        "SET RAC, h'dead\n"
        "SET R0, SaveCafe ; R0 = SaveCafe\n"
        "SET R1, SaveFood  ; R1 = SaveFood\n"
        "SET R2, h'dead\n"
        "SET R3, h'beef\n"
        "JE R2, R0 ; if R2(dead) == RAC(dead)JMP R0(SaveCafe)\n"
        "STOP\n"
        ":SaveCafe\n"
        "SET RAC, h'cafe\n"
        "STOP\n"
        ":SaveFood\n"
        "SET RAC, h'f00d\n"
        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 0xcafe);
}

TEST(TestProcessorPrograms, TestJNE)
{
    Assembler asmObj;
    std::string program =
        "; This program force tests JE and JNE by following the propoer jumps "
        "and ensuring the comply\n"
        "; RAC(dead)== R2(dead) RAC->cafe\n"
        "; RAC(dead)!= R2(dead) RAC->f00d\n"
        "SET RAC, h'beef\n"
        "SET R0, SaveCafe ; R0 = SaveCafe\n"
        "SET R1, SaveFood  ; R1 = SaveFood\n"
        "SET R2, h'dead\n"
        "SET R3, h'beef\n"
        "JNE R2, R1 ; if R2(dead) != RAC(beef)JMP R0(SaveFood)\n"
        ":SaveCafe\n"
        "SET RAC, h'cafe\n"
        "STOP\n"
        ":SaveFood\n"
        "SET RAC, h'f00d\n"
        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::RAC), 0xf00d);
}

TEST(TestProcessorPrograms, TestLOAD_STOR)
{
    Assembler asmObj;
    std::string program =
        "SET R0, h'1000 ; Set memory address in R0\n"
        "SET R1, h'cafe ; Set value to store\n"
        "STOR R1, R0 ; Store 0xcafe at address 0x1000\n"
        "SET R2, 0 ; Clear R2\n"
        "LOAD R0, R2 ; Load from address 0x1000 into R2\n"
        "STOP";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    TestProcessor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R1), 0xcafe);
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R2), 0xcafe);
    ASSERT_EQ(cpu.DereferenceRegisterRead(RegisterId::R0), 0xcafe);
}

using Memory16 = Memory<uint16_t>;
