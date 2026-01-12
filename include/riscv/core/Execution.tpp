#pragma once

#include <cstdint>
#include "riscv/core/Execution.hpp"
#include "riscv/core/Instruction.hpp"
#include "riscv/core/Trap.hpp"

// ============================================================
// Shift helper (RV32 masks to 5 bits)
// ============================================================

static inline uint32_t shamt(uint32_t v)
{
    return v & 31;
}
// ============================================================
// RISC-V Instruction Semantics
//
// This file implements the *architectural behavior* of each
// RV32IM instruction. All decoding is already done; this code
// executes the spec.
//
// Every instruction must:
//   - Read only architectural registers
//   - Write results only via set_reg()
//   - Update the PC exactly once
//
// This makes traps, syscalls, and single-stepping precise.
// ============================================================

template <size_t XLEN>
bool ExecutionEngine<XLEN>::execute(const DecodedInstruction &inst,
                                    uint32_t pc,
                                    State &state,
                                    Memory &memory)
{
    const uint32_t opcode = inst.opcode;
    const uint32_t rd = inst.rd;
    const uint32_t rs1 = inst.rs1;
    const uint32_t rs2 = inst.rs2;
    const uint32_t funct3 = inst.funct3;
    const uint32_t funct7 = inst.funct7;
    const int32_t imm = inst.imm;

    uint32_t next_pc = pc + 4;
    bool pc_written = false;

    switch (opcode)
    {
    case 0x37: // LUI
        state.set_reg(rd, (uint32_t)imm);
        break;

    case 0x17: // AUIPC
        state.set_reg(rd, pc + imm);
        break;

    case 0x6F: // JAL
        state.set_reg(rd, pc + 4);
        state.set_pc(pc + imm);
        pc_written = true;
        break;

    case 0x67: // JALR
    {
        uint32_t target = (state.reg(rs1) + imm) & ~1u;
        state.set_reg(rd, pc + 4);
        state.set_pc(target);
        pc_written = true;
        break;
    }

    case 0x63: // BRANCH
    {
        uint32_t a = state.reg(rs1);
        uint32_t b = state.reg(rs2);
        bool take = false;

        switch (funct3)
        {
        case 0x0:
            take = (a == b);
            break;
        case 0x1:
            take = (a != b);
            break;
        case 0x4:
            take = ((int32_t)a < (int32_t)b);
            break;
        case 0x5:
            take = ((int32_t)a >= (int32_t)b);
            break;
        case 0x6:
            take = (a < b);
            break;
        case 0x7:
            take = (a >= b);
            break;
        default:
            throw Trap{TrapCause::IllegalInstruction, pc, 0, inst.raw};
        }

        if (take)
        {
            state.set_pc(pc + imm);
            pc_written = true;
        }
        break;
    }

    case 0x03: // LOAD
    {
        uint32_t addr = state.reg(rs1) + imm;

        switch (funct3)
        {
        case 0x0: // LB
            state.set_reg(rd, (int8_t)memory.read_byte(addr));
            break;
        case 0x1: // LH
            state.set_reg(rd, (int16_t)memory.read_half(addr));
            break;
        case 0x2: // LW
            state.set_reg(rd, memory.read_word(addr));
            break;
        case 0x4: // LBU
            state.set_reg(rd, memory.read_byte(addr));
            break;
        case 0x5: // LHU
            state.set_reg(rd, memory.read_half(addr));
            break;
        default:
            throw Trap{TrapCause::IllegalInstruction, pc, 0, inst.raw};
        }
        break;
    }

    case 0x23: // STORE
    {
        uint32_t addr = state.reg(rs1) + imm;
        uint32_t val = state.reg(rs2);

        switch (funct3)
        {
        case 0x0: // SB
            memory.write_byte(addr, val);
            break;
        case 0x1: // SH
            memory.write_half(addr, val);
            break;
        case 0x2: // SW
            memory.write_word(addr, val);
            break;
        default:
            throw Trap{TrapCause::IllegalInstruction, pc, 0, inst.raw};
        }
        break;
    }

    case 0x13: // OP-IMM
    {
        uint32_t a = state.reg(rs1);

        switch (funct3)
        {
        case 0x0:
            state.set_reg(rd, a + imm);
            break;
        case 0x2:
            state.set_reg(rd, (int32_t)a < imm);
            break;
        case 0x3:
            state.set_reg(rd, a < (uint32_t)imm);
            break;
        case 0x4:
            state.set_reg(rd, a ^ imm);
            break;
        case 0x6:
            state.set_reg(rd, a | imm);
            break;
        case 0x7:
            state.set_reg(rd, a & imm);
            break;
        case 0x1:
            state.set_reg(rd, a << shamt(imm));
            break;
        case 0x5:
            state.set_reg(rd,
                          (funct7 & 0x20)
                              ? ((int32_t)a >> shamt(imm))
                              : (a >> shamt(imm)));
            break;
        default:
            throw Trap{TrapCause::IllegalInstruction, pc, 0, inst.raw};
        }
        break;
    }

    case 0x33: // OP / RV32M
    {
        uint32_t a = state.reg(rs1);
        uint32_t b = state.reg(rs2);

        if (funct7 == 0x01) // RV32M
        {
            int32_t s1 = (int32_t)a;
            int32_t s2 = (int32_t)b;
            uint32_t u1 = a;
            uint32_t u2 = b;

            switch (funct3)
            {
            case 0x0:
                state.set_reg(rd, (uint32_t)((int64_t)s1 * s2));
                break;
            case 0x1:
                state.set_reg(rd, (uint32_t)(((int64_t)s1 * s2) >> 32));
                break;
            case 0x2:
                state.set_reg(rd, (uint32_t)(((int64_t)s1 * u2) >> 32));
                break;
            case 0x3:
                state.set_reg(rd, (uint32_t)(((uint64_t)u1 * u2) >> 32));
                break;
            case 0x4:
                state.set_reg(rd,
                              b == 0 ? 0xFFFFFFFF : (s1 == INT32_MIN && s2 == -1) ? (uint32_t)INT32_MIN
                                                                                  : (uint32_t)(s1 / s2));
                break;
            case 0x5:
                state.set_reg(rd, b ? (u1 / u2) : 0xFFFFFFFF);
                break;
            case 0x6:
                state.set_reg(rd,
                              b == 0 ? a : (s1 == INT32_MIN && s2 == -1) ? 0
                                                                         : (uint32_t)(s1 % s2));
                break;
            case 0x7:
                state.set_reg(rd, b ? (u1 % u2) : a);
                break;
            default:
                throw Trap{TrapCause::IllegalInstruction, pc, 0, inst.raw};
            }
            break;
        }

        switch (funct3)
        {
        case 0x0:
            state.set_reg(rd, funct7 ? a - b : a + b);
            break;
        case 0x1:
            state.set_reg(rd, a << shamt(b));
            break;
        case 0x2:
            state.set_reg(rd, (int32_t)a < (int32_t)b);
            break;
        case 0x3:
            state.set_reg(rd, a < b);
            break;
        case 0x4:
            state.set_reg(rd, a ^ b);
            break;
        case 0x5:
            state.set_reg(rd,
                          funct7 ? ((int32_t)a >> shamt(b))
                                 : (a >> shamt(b)));
            break;
        case 0x6:
            state.set_reg(rd, a | b);
            break;
        case 0x7:
            state.set_reg(rd, a & b);
            break;
        default:
            throw Trap{TrapCause::IllegalInstruction, pc, 0, inst.raw};
        }
        break;
    }

    case 0x73:
        if (inst.is_ecall())
            throw Trap{TrapCause::Ecall, pc, 0, inst.raw};
        else
            throw Trap{TrapCause::IllegalInstruction, pc, 0, inst.raw};

    default:
        throw Trap{TrapCause::IllegalInstruction, pc, 0, inst.raw};
    }

    if (!pc_written)
        state.set_pc(next_pc);

    return true;
}
