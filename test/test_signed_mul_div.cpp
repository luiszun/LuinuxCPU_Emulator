#include <gtest/gtest.h>

#include "assembler.h"
#include "memory.h"
#include "processor.h"

using Memory16 = Memory<uint16_t>;

TEST(TestProcessorPrograms, TestSignedMulDiv)
{
    Assembler asmObj;
    // SMUL: -2 * 3 = -6
    // SMUL overflow: 16384 * 4 = 65536 -> overflow
    // SDIV: -9 / 3 = -3
    // SDIV overflow: INT16_MIN / -1 -> overflow
    std::string program =
        "SET R0, h'fffe\n"    // -2
        "SET R1, h'0003\n"    // 3
        "SMUL R0, R1, R10\n"  // R10 = -6
        "MOV R10, R9\n"       // Save result
        "MOV RFL, R3\n"       // Save flags after SMUL -6 (R3)

        "SET R0, h'4000\n"    // 16384
        "SET R1, h'0004\n"    // 4
        "SMUL R0, R1, R10\n"  // overflow case
        "MOV RFL, R8\n"       // Save flags after overflow

        "SET R0, h'fff7\n"    // -9
        "SET R1, h'0003\n"    // 3
        "SDIV R0, R1, R10\n"  // R10 = -3
        "MOV R10, R7\n"       // Save result
        "MOV RFL, R2\n"       // Save flags after SDIV -3 (R2)

        "SET R0, h'8000\n"    // INT16_MIN
        "SET R1, h'ffff\n"    // -1
        "SDIV R0, R1, R10\n"  // overflow case
        "MOV RFL, R6\n"       // Save flags after overflow

        "SET R0, h'0000\n"  // SMUL zero test: 0 * 3
        "SET R1, h'0003\n"
        "SMUL R0, R1, R10\n"
        "MOV R10, R5\n"
        "MOV RFL, R4\n"

        "SET R0, h'0002\n"  // SDIV by zero test
        "SET R1, h'0000\n"
        "SDIV R0, R1, R10\n"
        "MOV RFL, R1\n"

        "STOP\n";

    auto binProgram = asmObj.AssembleString(program);
    Memory16 programMemory(0x10000);
    programMemory.WritePayload(0, binProgram);
    Processor cpu(programMemory);
    cpu.ExecuteAll();

    // Check SMUL -6
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R9), static_cast<uint16_t>(static_cast<int16_t>(-6)));

    // Check flags after SMUL -6 (should be Negative set)
    FlagsObject f3(cpu.ReadRegister(RegisterId::R3));
    ASSERT_EQ(f3.flags.Zero, 0);
    ASSERT_EQ(f3.flags.Negative, 1);
    ASSERT_EQ(f3.flags.Exception, 0);

    // Check SMUL overflow flags
    FlagsObject f8(cpu.ReadRegister(RegisterId::R8));
    ASSERT_EQ(f8.flags.Overflow, 1);

    // Check SDIV -3
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R7), static_cast<uint16_t>(static_cast<int16_t>(-3)));

    // Check flags after SDIV -3 (Negative set)
    FlagsObject f2(cpu.ReadRegister(RegisterId::R2));
    ASSERT_EQ(f2.flags.Zero, 0);
    ASSERT_EQ(f2.flags.Negative, 1);
    ASSERT_EQ(f2.flags.Exception, 0);

    // Check SDIV overflow flags
    FlagsObject f6(cpu.ReadRegister(RegisterId::R6));
    ASSERT_EQ(f6.flags.Overflow, 1);

    // Check SMUL zero result and flags
    ASSERT_EQ(cpu.ReadRegister(RegisterId::R5), 0);
    FlagsObject f4(cpu.ReadRegister(RegisterId::R4));
    ASSERT_EQ(f4.flags.Zero, 1);
    ASSERT_EQ(f4.flags.Negative, 0);
    ASSERT_EQ(f4.flags.Exception, 0);

    // Check SDIV by zero sets exception
    FlagsObject f1(cpu.ReadRegister(RegisterId::R1));
    ASSERT_EQ(f1.flags.Exception, 1);
}
