#pragma once

#include <cstddef>
#include <ostream>
#include "riscv/core/State.hpp"
#include "riscv/memory/Memory.hpp"
#include "riscv/core/Execution.hpp"
#include "riscv/core/Instruction.hpp"
#include "riscv/platform/Syscall.hpp"

// CpuCore implements the CPU front-end:
//   - instruction fetch
//   - instruction decode
//   - trap and syscall dispatch
//
// It intentionally does NOT implement instruction semantics.
// Those live in ExecutionEngine, allowing precise control of
// architectural state and PC updates.

template <size_t XLEN>
class CpuCore
{
  public:
    using State = ArchitecturalState<XLEN>;
    using Memory = MemorySubsystem<XLEN>;

    CpuCore(State &s, Memory &m);

    bool step();

    void set_trace(bool enable)
    {
        trace = enable;
    }
    void set_trace_stream(std::ostream *o)
    {
        trace_out = o;
    }

    uint64_t get_inst_count() const
    {
        return inst_count;
    }

  private:
    State &state;
    Memory &memory;
    ExecutionEngine<XLEN> executor;
    SyscallHandler<State, Memory> syscall;

    bool trace = false;
    std::ostream *trace_out = nullptr;
    uint64_t inst_count = 0;

    DecodedInstruction fetch_and_decode();
};

#include "Processor.tpp"
