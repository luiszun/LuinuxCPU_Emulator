#include "assembler.h"
#include <gtest/gtest.h>

TEST(TestEncodeWord, BasicAssertions)
{
    Assembler asmObj;

    auto word = asmObj.EncodeInstructionWord("NOP");
    ASSERT_EQ(word, 0x7690);

    word = asmObj.EncodeInstructionWord("SHFL R10");
    ASSERT_EQ(word, 0x767f);

    word = asmObj.EncodeInstructionWord("MOV R0, R5");
    ASSERT_EQ(word, 0x725a);

    word = asmObj.EncodeInstructionWord("AND R0, R1, R2");
    ASSERT_EQ(word, 0x4567);

    word = asmObj.EncodeInstructionWord("SET R0, h'1010");
    ASSERT_EQ(word, 0x7625);

    EXPECT_ANY_THROW(asmObj.EncodeInstructionWord("POP R0 R0 R0"));
}

TEST(TestStringLiteral, BasicAssertions)
{
    Assembler asmObj;
    ASSERT_EQ(asmObj.GetValueFromStringLiteral("h'dead"), 0xdead);
    ASSERT_NE(asmObj.GetValueFromStringLiteral("h'beef"), 0x1337);
    ASSERT_EQ(asmObj.GetValueFromStringLiteral("h'1337"), 0x1337);
    ASSERT_EQ(asmObj.GetValueFromStringLiteral("1337"), 1337);
    ASSERT_EQ(asmObj.GetValueFromStringLiteral("-10"), static_cast<uint16_t>(-10));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("10.1"));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("- 10"));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("10'h"));
    EXPECT_ANY_THROW(asmObj.GetValueFromStringLiteral("0xdead"));
}