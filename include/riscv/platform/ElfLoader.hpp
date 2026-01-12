#pragma once

#include <cstdint>
#include <string>
#include "riscv/memory/Memory.hpp"
#include "riscv/core/State.hpp"

// ============================================================
// Minimal ELF32 loader for RV32 bare-metal programs
// ============================================================

// Loads a statically-linked RV32 ELF executable into the
// emulator address space.
//
// Responsibilities:
//   - parse ELF headers
//   - map PT_LOAD segments at their virtual addresses
//   - zero-initialize BSS
//   - apply RISC-V relocations
//   - set the initial PC
//
// This allows unmodified toolchain output to run directly.

class ElfLoader
{
  public:
    static bool load(const std::string &path,
                     MemorySubsystem<32> &memory,
                     ArchitecturalState<32> &state);
};
