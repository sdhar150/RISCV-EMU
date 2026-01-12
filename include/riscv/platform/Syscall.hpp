#pragma once

#include <cstdint>

// ============================================================
// SyscallHandler
//
// Implements a minimal Linux-like kernel ABI for user programs.
// This layer defines the boundary between:
//   user mode (RISC-V code)
//   kernel mode (the emulator runtime)
//
// It provides enough functionality to support newlib:
//   heap, I/O, and process exit.
// ============================================================

template <typename State, typename Memory>
class SyscallHandler
{
  public:
    explicit SyscallHandler(Memory &mem)
        : memory(mem), program_break(0), mmap_top(0)
    {
    }

    bool handle(State &state);

  private:
    Memory &memory;
    uint32_t program_break; // current end of heap (brk)
    uint32_t mmap_top;
};

#include "Syscall.tpp"
