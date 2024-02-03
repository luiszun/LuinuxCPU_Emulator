#include "assembler.h"
#include <gtest/gtest.h>

class TestAssembler : public Assembler
{
  public:
    bool HasPendingLiteral()
    {
        return _pendingLiteralValue;
    }
    uint16_t GetPendingLiteral()
    {
        return _literalValue;
    }

    bool ContainsInstruction(std::string line)
    {
        return _ContainsInstruction(line);
    }
};

TEST(TestAssemblerSuite, TestInstructionIdentification)
{
    TestAssembler tstAsm;

    ASSERT_EQ(tstAsm.ContainsInstruction(""), false);
    ASSERT_EQ(tstAsm.ContainsInstruction("       "), false);
    ASSERT_EQ(tstAsm.ContainsInstruction("; hello"), false);
    ASSERT_EQ(tstAsm.ContainsInstruction("MOV R0, R2 ; hello"), true);
}
TEST(TestAssemblerSuite, TestEncodeWord)
{
    Assembler asmObj;

    auto word = asmObj.EncodeInstructionWord("NOP");
    ASSERT_EQ(word, 0x7690);

    word = asmObj.EncodeInstructionWord("SHFL R10 ; A shift with a comment");
    ASSERT_EQ(word, 0x767f);

    word = asmObj.EncodeInstructionWord("MOV R0, R5 ");
    ASSERT_EQ(word, 0x725a);

    word = asmObj.EncodeInstructionWord("AND R0, R1, R2");
    ASSERT_EQ(word, 0x4567);

    word = asmObj.EncodeInstructionWord("SET R0, h'1010");
    ASSERT_EQ(word, 0x7625);

    word = asmObj.EncodeInstructionWord("SET R0, h'1010");
    ASSERT_EQ(word, 0x7625);

    EXPECT_ANY_THROW(asmObj.EncodeInstructionWord("POP R0, R0, R0"));
    EXPECT_ANY_THROW(asmObj.EncodeInstructionWord("SET R0, 0xff"));

    TestAssembler tstAsm;

    word = tstAsm.EncodeInstructionWord("goto:R0", 2);
    ASSERT_EQ(word, 0x7625);
    ASSERT_EQ(tstAsm.HasPendingLiteral(), true);
    ASSERT_EQ(tstAsm.GetPendingLiteral(), 6);
}

TEST(TestAssemblerSuite, TestStringLiteral)
{
    Assembler asmObj;
    ASSERT_EQ(asmObj.GetValueFromStringLiteral("h'dead"), 0xdead);
    ASSERT_NE(asmObj.GetValueFromStringLiteral("h'beef"), 0x1337);
    ASSERT_EQ(asmObj.GetValueFromStringLiteral("h'1337"), 0x1337);
    ASSERT_EQ(asmObj.GetValueFromStringLiteral("1337"), 1337);
    ASSERT_EQ(asmObj.GetValueFromStringLiteral("-10"), static_cast<uint16_t>(-10));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("h'"));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("10.1"));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("- 10"));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("10'h"));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("0xdead"));
}

TEST(TestAssemblerSuite, TestMultilineProgram)
{
    Assembler asmObj;
    std::string program = "SET R0, h'100\nSET R1, h'001\nAND R0, R1, R2";

    // Because any arm, amd64 or x86 are little endian, write in little endian.
    std::vector<uint16_t> expectedBinary{0x2576, 0x0001, 0x2676, 0x0100, 0x6745};

    auto binProgram = asmObj.AssembleString(program);
    ASSERT_EQ(expectedBinary, binProgram);
}

TEST(TestAssemblerSuite, TestGoto)
{
    Assembler asmObj;
    std::string program = "SET R0, h'100\nSET R1, h'001\nAND R0, R1, R2\ngoto:R0";

    // Because any arm, amd64 or x86 are little endian, write in little endian.
    std::vector<uint16_t> expectedBinary{0x2576, 0x0001, 0x2676, 0x0100, 0x6745, 0x2576, 0x0e00};

    auto binProgram = asmObj.AssembleString(program);
    ASSERT_EQ(expectedBinary, binProgram);
}

TEST(TestAssemblerSuite, TestEmptyLinesAndCommentLines)
{
    Assembler asmObj;
    std::string program = "\n\nSET R0, h'100\n\nSET R1, h'001;cool comment\n;lalalala\n\n\nAND R0, R1, R2\n";

    // Because any arm, amd64 or x86 are little endian, write in little endian.
    std::vector<uint16_t> expectedBinary{0x2576, 0x0001, 0x2676, 0x0100, 0x6745};

    auto binProgram = asmObj.AssembleString(program);
    ASSERT_EQ(expectedBinary, binProgram);
}

TEST(TestAssemblerSuite, TestLoop10x)
{
    Assembler asmObj;
    std::string program = "SET R0, 10 ; This is the number of times it will loop\nSET R10, 0 ; Initialize R10 to be "
                          "our counter\n\ngoto:R2 ; loop on R2\nINC R10\nSUB R0, R10, R1\nJNZ R1, R2\n";

    // Because any arm, amd64 or x86 are little endian, write in little endian.
    std::vector<uint16_t> expectedBinary{0x2576, 0x0a00, 0x2f76, 0x0000, 0x2776, 0x0c00, 0x8f76, 0xf615, 0x6771};

    auto binProgram = asmObj.AssembleString(program);

    auto str = asmObj.GetAssembledPayloadHex();
    ASSERT_EQ(expectedBinary, binProgram);
}