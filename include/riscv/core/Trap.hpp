#pragma once

#include <cstdint>

// ============================================================
// RISC-V Trap Model
// ============================================================
//
// Represents synchronous exceptions such as:
//   - illegal instructions
//   - memory faults
//   - ECALL
//
// Traps carry the faulting PC and address, allowing
// precise exception handling just like real hardware.

enum class TrapCause
{
    IllegalInstruction,
    LoadAccessFault,
    StoreAccessFault,
    MisalignedAccess,
    Ecall
};

struct Trap
{
    TrapCause cause; // RISC-V trap reason
    uint32_t pc;     // PC of faulting instruction
    uint32_t addr;   // Faulting memory address (0 if not applicable)
    uint32_t inst;   // Raw instruction (0 if not applicable)

    Trap(TrapCause c, uint32_t p, uint32_t a, uint32_t i)
        : cause(c), pc(p), addr(a), inst(i)
    {
    }
};
