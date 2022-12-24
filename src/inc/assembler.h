#pragma once
#include "common.h"
#include "opcode.h"
#include "register.h"

class Assembler
{
  public:
    Assembler(std::string codeFilename, std::string binaryFilename)
        : _inFilename{codeFilename}, _outFilename{binaryFilename}, _canWriteFiles{true}
    {
    }

    Assembler()
    {
    }

    uint16_t EncodeInstructionWord(const OpCode &opCode, const std::array<RegisterId, 3> &args);
    uint16_t EncodeInstructionWord(std::string instruction, uint16_t instIndex = 0);
    uint16_t GetValueFromStringLiteral(std::string literal);
    std::vector<uint16_t> AssembleFile();
    void WriteBinaryFile(std::vector<uint16_t> &program);

  protected:
    uint16_t _WordToBigEndian(uint16_t word);
    bool _IsSpecialInstruction(OpCodeId id);
    std::string _RemoveComments(std::string str);

    std::string _inFilename;
    std::string _outFilename;
    std::ifstream _inFileStream;
    std::ofstream _outFileStream;
    uint16_t _literalValue = 0;
    bool _pendingLiteralValue = false;
    bool _canWriteFiles = false;
    std::unordered_map<std::string, uint16_t> tagAddressMap;
};
