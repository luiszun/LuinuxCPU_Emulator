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
    uint16_t GetValueFromStringLiteral(std::string literal) const;
    std::vector<uint8_t> AssembleFile();
    std::vector<uint8_t> AssembleString(std::string program);
    void WriteBinaryFile(std::vector<uint8_t> &program, bool stdOutPayload = false);
    std::string GetAssembledPayloadHex() const;

  protected:
    uint16_t _WordToBigEndian(uint16_t word) const;
    bool _IsSpecialInstruction(OpCodeId id) const;
    std::string _RemoveComments(std::string str) const;
    bool _ContainsInstruction(std::string line) const;

    std::vector<uint8_t> _assembledPayload;
    std::string _inFilename;
    std::string _outFilename;
    std::ifstream _inFileStream;
    std::ofstream _outFileStream;
    uint16_t _literalValue = 0;
    bool _pendingLiteralValue = false;
    bool _canWriteFiles = false;
    std::unordered_map<std::string, uint16_t> tagAddressMap;
};
