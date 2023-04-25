#pragma once
#include "memory.h"
#include "opcode.h"
#include "register.h"

// 256 bytes of internal memory, used for 8x register banks
constexpr size_t InternalMemorySize = 256;
constexpr uint16_t RSP_DefaultAddress = 0xffff - 512;

using Memory16 = Memory<uint16_t>;
using NVMemory16 = NVMemory<uint16_t>;
using Memory8 = Memory<uint8_t>;

enum class InstructionCycle
{
    Idle = 0,
    Decode,
    Fetch,
    Execute
};

class Processor
{
  public:
    Processor(Memory16 &programMemory);

    void WriteRegister(RegisterId reg, uint16_t value);
    uint16_t ReadRegister(RegisterId reg) const;

    void PerformExecutionCycle();

    // TODO:
    // execute
    // alu
    // flags updates
    // pause

  protected:
    uint16_t _ReadMemoryWord(Memory16 &memory, uint16_t address) const;
    void _WriteMemoryWord(Memory16 &memory, uint16_t address, uint16_t value);
    void _FetchInstruction();
    void _DecodeInstruction();
    void _ExecuteInstruction();
    void _CleanInstructionCycle();

    Memory16 &_programMemory;
    Memory16 _mainMemory;
    Memory8 _intrMem;
    std::unordered_map<RegisterId, Register> _registers;

    OpCodeId _decodedOpCodeId = OpCodeId::INVALID_INSTR;
    uint16_t _literalValue = 0;
    uint16_t _fetchedInstruction = 0;
    std::vector<RegisterId> _instructionArgs;
    uint16_t _2wordOperand;
    InstructionCycle _instructionStatus = InstructionCycle::Idle;
};
