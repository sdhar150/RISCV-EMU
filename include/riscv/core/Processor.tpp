#pragma once

#include <iostream>
#include "riscv/core/Instruction.hpp"
#include "riscv/platform/Syscall.hpp"
#include "riscv/core/Trap.hpp"

// ------------------------------------------------------------
// CpuCore constructor
// ------------------------------------------------------------

template <size_t XLEN>
CpuCore<XLEN>::CpuCore(State &state, Memory &memory)
    : state(state),
      memory(memory),
      executor(),
      syscall(memory)
{
}

// ------------------------------------------------------------
// Fetch + decode
// ------------------------------------------------------------

template <size_t XLEN>
DecodedInstruction CpuCore<XLEN>::fetch_and_decode()
{
    uint32_t pc = state.pc;
    if (pc & 3)
        throw Trap{TrapCause::MisalignedAccess, pc, pc, 0};

    uint32_t raw = memory.read_word(pc);
    return decode_instruction(raw);
}

// ------------------------------------------------------------
// Trace helper
// ------------------------------------------------------------

template <size_t XLEN>
inline void print_trace(std::ostream *out,
                        uint32_t pc,
                        const DecodedInstruction &d)
{
    if (!out)
        return;

    (*out) << "PC=0x" << std::hex << pc
           << " INST=0x" << d.raw
           << " rd=" << std::dec << (int)d.rd
           << " rs1=" << (int)d.rs1
           << " rs2=" << (int)d.rs2
           << " imm=" << d.imm
           << "\n";
}
// ------------------------------------------------------------
// Execute one instruction
// ------------------------------------------------------------
// Execute exactly one architectural instruction.
// This function models a precise CPU:
//   - instructions either complete or trap
//   - ECALL is implemented as a synchronous exception
//   - PC always reflects the faulting instruction on traps

template <size_t XLEN>
bool CpuCore<XLEN>::step()
{
    try
    {
        uint32_t pc = state.pc;
        DecodedInstruction inst = fetch_and_decode();

        if (trace)
            print_trace<XLEN>(trace_out, pc, inst);

        executor.execute(inst, pc, state, memory);
        inst_count++;
        return true;
    }
    catch (const Trap &t)
    {
        if (t.cause == TrapCause::Ecall)
        {
            bool cont = syscall.handle(state);
            state.set_pc(t.pc + 4); // resume after ecall
            return cont;
        }

        std::cerr << "\n=== CPU TRAP ===\n";
        std::cerr << "PC      = 0x" << std::hex << t.pc << "\n";
        std::cerr << "Cause   = " << static_cast<int>(t.cause) << "\n";
        std::cerr << "Address = 0x" << std::hex << t.addr << "\n";
        std::cerr << "Inst    = 0x" << std::hex << t.inst << "\n";
        std::cerr << "Instructions executed: " << inst_count << "\n";
        std::cerr << "=============\n";
        return false;
    }
}
