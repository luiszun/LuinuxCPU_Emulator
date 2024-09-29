#pragma once
#include "memory.h"
#include "opcode.h"
#include "register.h"

// 256 bytes of internal memory, used for 8x register banks
constexpr size_t InternalMemorySize = 256;
constexpr size_t MainMemorySize = 0x10000;
constexpr uint16_t RSP_DefaultAddress = 0xffff - 512;

using Memory16 = Memory<uint16_t>;
using NVMemory16 = NVMemory<uint16_t>;
using Memory8 = Memory<uint8_t>;

typedef std::pair<uint16_t, uint16_t> ConstantPair;
enum class InstructionCycle
{
    Idle = 0,
    Decode,
    Fetch,
    Execute,
    Halted
};

class Processor
{
  public:
    Processor(Memory16 &programMemory, std::shared_ptr<NVMemory16> nvram = nullptr);

    void WriteRegister(RegisterId reg, uint16_t value);
    uint16_t ReadRegister(RegisterId reg) const;

    void PerformExecutionCycle();
    void ExecuteAll()
    {

        while (_instructionStatus != InstructionCycle::Halted)
        {
            FlagsObject f(ReadRegister(RegisterId::RFL));
            if (f.flags.Trap == 1)
            {
                break;
            }
            _DoPerformExecutionCycle();
        }
    }

    // TODO:
    // execute
    // alu
    // flags updates
    // pause

  protected:
    void _DoPerformExecutionCycle();
    void _FetchInstruction();
    void _DecodeInstruction();
    void _ExecuteInstruction();
    void _CleanInstructionCycle();
    uint16_t _DereferenceRegisterRead(RegisterId reg) const;
    void _DereferenceRegisterWrite(RegisterId reg, uint16_t value);

    // All the instructions!

    // TODO: Org some of these into ALU
    ConstantPair _Get_RR(std::vector<std::shared_ptr<Register>> args) const;
    ConstantPair _Get_MR(std::vector<std::shared_ptr<Register>> args) const;
    ConstantPair _Get_RM(std::vector<std::shared_ptr<Register>> args) const;
    ConstantPair _Get_MM(std::vector<std::shared_ptr<Register>> args) const;
    void _Base_ADD(ConstantPair values, std::shared_ptr<Register> dest);
    void _Base_SUB(ConstantPair values, std::shared_ptr<Register> dest);
    void _Base_MUL(ConstantPair values, std::shared_ptr<Register> dest);
    void _Base_DIV(ConstantPair values, std::shared_ptr<Register> dest);
    void _Base_AND(ConstantPair values, std::shared_ptr<Register> dest);
    void _Base_OR(ConstantPair values, std::shared_ptr<Register> dest);
    void _Base_XOR(ConstantPair values, std::shared_ptr<Register> dest);
    void _Base_JZ(ConstantPair values);
    void _Base_JNZ(ConstantPair values);
    void ADD(std::vector<std::shared_ptr<Register>> args);
    void SUB(std::vector<std::shared_ptr<Register>> args);
    void MUL(std::vector<std::shared_ptr<Register>> args);
    void STOP(std::vector<std::shared_ptr<Register>> args);
    void SET(std::vector<std::shared_ptr<Register>> args);
    void DIV(std::vector<std::shared_ptr<Register>> args);
    void AND(std::vector<std::shared_ptr<Register>> args);
    void OR(std::vector<std::shared_ptr<Register>> args);
    void XOR(std::vector<std::shared_ptr<Register>> args);
    void JZ(std::vector<std::shared_ptr<Register>> args);
    void JNZ(std::vector<std::shared_ptr<Register>> args);
    void JE(std::vector<std::shared_ptr<Register>> args);
    void JNE(std::vector<std::shared_ptr<Register>> args);
    void MOV(std::vector<std::shared_ptr<Register>> args);
    void TSTB(std::vector<std::shared_ptr<Register>> args);
    void SETZ(std::vector<std::shared_ptr<Register>> args);
    void SETO(std::vector<std::shared_ptr<Register>> args);
    void PUSH(std::vector<std::shared_ptr<Register>> args);
    void POP(std::vector<std::shared_ptr<Register>> args);
    void NOT(std::vector<std::shared_ptr<Register>> args);
    void SHFR(std::vector<std::shared_ptr<Register>> args);
    void SHFL(std::vector<std::shared_ptr<Register>> args);
    void INC(std::vector<std::shared_ptr<Register>> args);
    void DEC(std::vector<std::shared_ptr<Register>> args);
    void NOP(std::vector<std::shared_ptr<Register>> args);
    void ADD_RM(std::vector<std::shared_ptr<Register>> args);
    void ADD_MR(std::vector<std::shared_ptr<Register>> args);
    void ADD_MM(std::vector<std::shared_ptr<Register>> args);
    void SUB_RM(std::vector<std::shared_ptr<Register>> args);
    void SUB_MR(std::vector<std::shared_ptr<Register>> args);
    void SUB_MM(std::vector<std::shared_ptr<Register>> args);
    void MUL_RM(std::vector<std::shared_ptr<Register>> args);
    void MUL_MR(std::vector<std::shared_ptr<Register>> args);
    void MUL_MM(std::vector<std::shared_ptr<Register>> args);
    void DIV_RM(std::vector<std::shared_ptr<Register>> args);
    void DIV_MR(std::vector<std::shared_ptr<Register>> args);
    void DIV_MM(std::vector<std::shared_ptr<Register>> args);
    void AND_RM(std::vector<std::shared_ptr<Register>> args);
    void AND_MR(std::vector<std::shared_ptr<Register>> args);
    void AND_MM(std::vector<std::shared_ptr<Register>> args);
    void OR_RM(std::vector<std::shared_ptr<Register>> args);
    void OR_MR(std::vector<std::shared_ptr<Register>> args);
    void OR_MM(std::vector<std::shared_ptr<Register>> args);
    void XOR_RM(std::vector<std::shared_ptr<Register>> args);
    void XOR_MR(std::vector<std::shared_ptr<Register>> args);
    void XOR_MM(std::vector<std::shared_ptr<Register>> args);
    void JZ_RM(std::vector<std::shared_ptr<Register>> args);
    void JZ_MR(std::vector<std::shared_ptr<Register>> args);
    void JZ_MM(std::vector<std::shared_ptr<Register>> args);
    void JNZ_RM(std::vector<std::shared_ptr<Register>> args);
    void JNZ_MR(std::vector<std::shared_ptr<Register>> args);
    void JNZ_MM(std::vector<std::shared_ptr<Register>> args);
    void MOV_RM(std::vector<std::shared_ptr<Register>> args);
    void MOV_MR(std::vector<std::shared_ptr<Register>> args);
    void MOV_MM(std::vector<std::shared_ptr<Register>> args);
    void TSTB_M(std::vector<std::shared_ptr<Register>> args);
    void SETZ_M(std::vector<std::shared_ptr<Register>> args);
    void SETO_M(std::vector<std::shared_ptr<Register>> args);
    void SET_M(std::vector<std::shared_ptr<Register>> args);
    void PUSH_M(std::vector<std::shared_ptr<Register>> args);
    void POP_M(std::vector<std::shared_ptr<Register>> args);
    void NOT_M(std::vector<std::shared_ptr<Register>> args);
    void SHFR_M(std::vector<std::shared_ptr<Register>> args);
    void SHFL_M(std::vector<std::shared_ptr<Register>> args);
    void INC_M(std::vector<std::shared_ptr<Register>> args);
    void DEC_M(std::vector<std::shared_ptr<Register>> args);
    void TRAP(std::vector<std::shared_ptr<Register>> args);
    void SWM(std::vector<std::shared_ptr<Register>> args);
    void JMP(std::vector<std::shared_ptr<Register>> args);

    Memory16 &_programMemory;
    std::shared_ptr<Memory16> _mainMemory;
    std::shared_ptr<Memory16> _sram;
    std::shared_ptr<NVMemory16> _nvram;
    Memory8 _internalMemory;
    std::unordered_map<RegisterId, Register> _registers;

    OpCodeId _decodedOpCodeId = OpCodeId::INVALID_INSTR;
    uint16_t _literalValue = 0;
    uint16_t _fetchedInstruction = 0;
    std::vector<RegisterId> _instructionArgs;
    uint16_t _2wordOperand;
    InstructionCycle _instructionStatus = InstructionCycle::Idle;
};
