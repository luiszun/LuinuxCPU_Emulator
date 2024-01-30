    // ALU operations ADD, SUB, MIL, DIV, AND, OR, XOR, NOT
    // ALU-masked ops - INC, DEC
    // Branching ops - JZ, JNZ
    // Specific tasks - SET, SETZ, SETO, TSTB, PUSH, POP, NOP, STOP
    // Data movement - MOV, LOAD, STOR

#include "register.h"

void ADD(RegisterId OpA, RegisterId OpB,RegisterId OpC = RegisterId::END_OF_REGLIST)
{
// Simple ADD
if(OpC == RegisterId::END_OF_REGLIST)
{

}
}
  /* 
   ADD = 0,
    SUB,
    MUL,
    DIV,
    AND,
    OR,
    XOR,
    JZ,
    JNZ,
    MOV,
    LOAD,
    STOR,
    TSTB,
    SETZ,
    SETO,
    SET,
    PUSH,
    POP,
    NOT,
    SHFR,
    SHFL,
    INC,
    DEC,
    NOP,
    STOP,
    ADD_RM,
    ADD_MR,
    ADD_MM,
    SUB_RM,
    SUB_MR,
    SUB_MM,
    MUL_RM,
    MUL_MR,
    MUL_MM,
    DIV_RM,
    DIV_MR,
    DIV_MM,
    AND_RM,
    AND_MR,
    AND_MM,
    OR_RM,
    OR_MR,
    OR_MM,
    XOR_RM,
    XOR_MR,
    XOR_MM,
    JZ_RM,
    JZ_MR,
    JZ_MM,
    JNZ_RM,
    JNZ_MR,
    JNZ_MM,
    MOV_RM,
    MOV_MR,
    MOV_MM,
    TSTB_M,
    SETZ_M,
    SETO_M,
    SET_M,
    PUSH_M,
    POP_M,
    NOT_M,
    SHFR_M,
    SHFL_M,
    INC_M,
    DEC_M,
    INVALID_INSTR
    */