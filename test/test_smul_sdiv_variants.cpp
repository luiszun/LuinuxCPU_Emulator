#include <gtest/gtest.h>

#include "assembler.h"
#include "memory.h"
#include "processor.h"
#include "utils.h"

using Memory16 = Memory<uint16_t>;

TEST(TestSMULSDIVAssembler, SMUL_RR_basic)
{
    Assembler asmObj;

    std::string program =
        "SET R1, h'0000 ; flags backup\n"
        "SET R5, h'fffe ; -2\n"
        "SET R6, 3\n"
        "SMUL R5, R6, R10\n"
        "MOV RFL, R1\n"
        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    Processor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R10), static_cast<uint16_t>(static_cast<int16_t>(-6)));
    FlagsObject f(cpu.ReadRegister(RegisterId::R1));
    ASSERT_EQ(f.flags.Exception, 0);
    ASSERT_EQ(f.flags.Negative, 1);
}

TEST(TestSMULSDIVAssembler, SDIV_RR_basic)
{
    Assembler asmObj;

    std::string program =
        "SET R1, h'0000 ; flags backup\n"
        "SET R5, h'fff7 ; -9\n"
        "SET R6, 3\n"
        "SDIV R5, R6, R10\n"
        "MOV RFL, R1\n"
        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    Processor cpu(programMemory);
    cpu.ExecuteAll();

    ASSERT_EQ(cpu.ReadRegister(RegisterId::R10), static_cast<uint16_t>(static_cast<int16_t>(-3)));
    FlagsObject f(cpu.ReadRegister(RegisterId::R1));
    ASSERT_EQ(f.flags.Exception, 0);
    ASSERT_EQ(f.flags.Negative, 1);
}

TEST(TestSMULSDIVAssembler, SDIV_RR_divide_by_zero_sets_exception)
{
    Assembler asmObj;

    std::string program =
        "SET R1, h'0000 ; flags backup\n"
        "SET R5, 2\n"
        "SET R6, 0\n"
        "SDIV R5, R6, R10\n"
        "MOV RFL, R1\n"
        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    Processor cpu(programMemory);
    cpu.ExecuteAll();

    FlagsObject f(cpu.ReadRegister(RegisterId::R1));
    ASSERT_EQ(f.flags.Exception, 1);
}

TEST(TestSMULSDIVAssembler, SDIV_RR_overflow)
{
    Assembler asmObj;

    std::string program =
        "SET R1, h'0000 ; flags backup\n"
        "SET R5, h'8000 ; INT16_MIN\n"
        "SET R6, h'ffff ; -1\n"
        "SDIV R5, R6, R10\n"
        "MOV RFL, R1\n"
        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    Processor cpu(programMemory);
    cpu.ExecuteAll();

    FlagsObject f(cpu.ReadRegister(RegisterId::R1));
    ASSERT_EQ(f.flags.Overflow, 1);
}

