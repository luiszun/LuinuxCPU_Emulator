#include <gtest/gtest.h>

#include "assembler.h"
#include "memory.h"
#include "processor.h"
#include "utils.h"

using Memory16 = Memory<uint16_t>;

TEST(TestAluFlags, TestAluFlags_ADD)
{
    Assembler asmObj;

    std::string program =
        "SET R1, h'0000 ; R1 will hold flags after zero-result ADD\n"
        "SET R2, h'0000 ; R2 will hold flags after negative-result ADD\n"
        "SET R3, h'0000 ; R3 will hold flags after carry-result ADD\n"
        "SET R4, h'0000 ; R4 will hold flags after overflow-result ADD\n"

        "SET R5, h'0000 ; Zero test: 0 + 0 = 0\n"
        "SET R6, h'0000\n"
        "ADD R5, R6, R10 ; Expected: Zero set, Negative clear, Carry clear, "
        "Overflow clear\n"
        "MOV RFL, R1 ; Backup flags for zero test\n"

        "SET R5, h'8000 ; Negative test: h'8000 + 0 = h'8000\n"
        "SET R6, h'0000\n"
        "ADD R5, R6, R10 ; Expected: Negative set, Zero clear, Carry clear, "
        "Overflow clear\n"
        "MOV RFL, R2 ; Backup flags for negative test\n"

        "SET R5, h'ffff ; Carry test: h'ffff + 1 = h'0000 with unsigned carry\n"
        "SET R6, h'0001\n"
        "ADD R5, R6, R10 ; Expected: Carry set, Zero set, Negative clear, "
        "Overflow clear\n"
        "MOV RFL, R3 ; Backup flags for carry test\n"

        "SET R5, h'7fff ; Overflow test: signed 32767 + 1 = signed overflow\n"
        "SET R6, h'0001\n"
        "ADD R5, R6, R10 ; Expected: Overflow set, Negative set, Zero clear, "
        "Carry clear\n"
        "MOV RFL, R4 ; Backup flags for overflow test\n"

        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    auto escapedProgram = HexEscapedString(binProgram);
    std::cout << "Assembled Program (Hex Escaped): " << escapedProgram << std::endl;

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    Processor cpu(programMemory);
    cpu.ExecuteAll();

    // Check zero flag for zero result case
    FlagsObject f1(cpu.ReadRegister(RegisterId::R1));
    ASSERT_EQ(f1.flags.Zero, 1);
    ASSERT_EQ(f1.flags.Negative, 0);
    ASSERT_EQ(f1.flags.Carry, 0);
    ASSERT_EQ(f1.flags.Overflow, 0);

    // Check negative flag for negative result case
    FlagsObject f2(cpu.ReadRegister(RegisterId::R2));
    ASSERT_EQ(f2.flags.Zero, 0);
    ASSERT_EQ(f2.flags.Negative, 1);
    ASSERT_EQ(f2.flags.Carry, 0);
    ASSERT_EQ(f2.flags.Overflow, 0);

    // Check carry flag for unsigned overflow/wraparound case
    FlagsObject f3(cpu.ReadRegister(RegisterId::R3));
    ASSERT_EQ(f3.flags.Zero, 1);
    ASSERT_EQ(f3.flags.Negative, 0);
    ASSERT_EQ(f3.flags.Carry, 1);
    ASSERT_EQ(f3.flags.Overflow, 0);

    // Check overflow flag for signed overflow case
    FlagsObject f4(cpu.ReadRegister(RegisterId::R4));
    ASSERT_EQ(f4.flags.Zero, 0);
    ASSERT_EQ(f4.flags.Negative, 1);
    ASSERT_EQ(f4.flags.Carry, 0);
    ASSERT_EQ(f4.flags.Overflow, 1);
}


TEST(TestAluFlags, TestAluFlags_SUB)
{
    Assembler asmObj;

    std::string program =
        "SET R1, h'0000 ; R1 will hold flags after zero-result SUB\n"
        "SET R2, h'0000 ; R2 will hold flags after negative-result SUB\n"
        "SET R3, h'0000 ; R3 will hold flags after carry-result SUB\n"
        "SET R4, h'0000 ; R4 will hold flags after overflow-result SUB\n"

        "SET R5, h'0000 ; Zero test: 0 - 0 = 0\n"
        "SET R6, h'0000\n"
        "SUB R5, R6, R10 ; Expected: Zero set, Negative clear, Carry clear, Overflow clear\n"
        "MOV RFL, R1 ; Backup flags for zero test\n"

        "SET R5, h'8000 ; Negative test: h'8000 - 0 = h'8000\n"
        "SET R6, h'0000\n"
        "SUB R5, R6, R10 ; Expected: Negative set, Zero clear, Carry clear, Overflow clear\n"
        "MOV RFL, R2 ; Backup flags for negative test\n"

        "SET R5, h'0000 ; Carry test: 0 - 1 = h'ffff with borrow\n"
        "SET R6, h'0001\n"
        "SUB R5, R6, R10 ; Expected: Carry set, Negative set, Zero clear, Overflow clear\n"
        "MOV RFL, R3 ; Backup flags for carry test\n"

        "SET R5, h'8000 ; Overflow test: h'8000 - 1 = h'7fff signed overflow\n"
        "SET R6, h'0001\n"
        "SUB R5, R6, R10 ; Expected: Overflow set, Negative clear, Zero clear, Carry clear\n"
        "MOV RFL, R4 ; Backup flags for overflow test\n"

        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);

    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    Processor cpu(programMemory);
    cpu.ExecuteAll();

    // Check zero flag for zero result case
    FlagsObject f1(cpu.ReadRegister(RegisterId::R1));
    ASSERT_EQ(f1.flags.Zero, 1);
    ASSERT_EQ(f1.flags.Negative, 0);
    ASSERT_EQ(f1.flags.Carry, 0);
    ASSERT_EQ(f1.flags.Overflow, 0);

    // Check negative flag for negative result case
    FlagsObject f2(cpu.ReadRegister(RegisterId::R2));
    ASSERT_EQ(f2.flags.Zero, 0);
    ASSERT_EQ(f2.flags.Negative, 1);
    ASSERT_EQ(f2.flags.Carry, 0);
    ASSERT_EQ(f2.flags.Overflow, 0);

    // Check carry/borrow flag for underflow case
    FlagsObject f3(cpu.ReadRegister(RegisterId::R3));
    ASSERT_EQ(f3.flags.Zero, 0);
    ASSERT_EQ(f3.flags.Negative, 1);
    ASSERT_EQ(f3.flags.Carry, 1);
    ASSERT_EQ(f3.flags.Overflow, 0);

    // Check overflow flag for signed overflow case
    FlagsObject f4(cpu.ReadRegister(RegisterId::R4));
    ASSERT_EQ(f4.flags.Zero, 0);
    ASSERT_EQ(f4.flags.Negative, 0);
    ASSERT_EQ(f4.flags.Carry, 0);
    ASSERT_EQ(f4.flags.Overflow, 1);
}
