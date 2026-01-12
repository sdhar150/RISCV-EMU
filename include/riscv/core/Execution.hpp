#pragma once

#include "riscv/core/State.hpp"
#include "riscv/memory/Memory.hpp"
#include "riscv/core/Instruction.hpp"

// ============================================================
// ExecutionEngine
//
// Implements the architectural semantics of the RISC-V RV32IM ISA.
// Given a fully-decoded instruction and the current PC, it:
//
//   - Computes ALU results
//   - Performs loads and stores via the memory subsystem
//   - Updates architectural registers
//   - Computes the next PC (including branches and jumps)
//
// The CpuCore owns the fetch/decode/trap loop.
// The ExecutionEngine owns all instruction semantics.
//
// This separation mirrors real CPU microarchitecture:
//   front-end (fetch/decode) vs execution backend.
// ============================================================

template <size_t XLEN>
class ExecutionEngine
{
  public:
    using State = ArchitecturalState<XLEN>;
    using Memory = MemorySubsystem<XLEN>;

    bool execute(const DecodedInstruction &inst,
                 uint32_t pc,
                 State &state,
                 Memory &memory);
};

#include "Execution.tpp" // ⬅️ template implementation
