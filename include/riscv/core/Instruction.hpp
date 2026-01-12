#pragma once

#include <cstdint>

// ============================================================
// Decoded RISC-V instruction (RV32)
// ============================================================
// This struct represents a *fully decoded* instruction.
// The raw bits are decoded exactly once during fetch.
// Execution logic never re-extracts fields from the raw word.
//
// This mirrors real pipelines: decode is separate from execute.

struct DecodedInstruction
{
    uint32_t raw = 0;

    uint8_t opcode = 0;
    uint8_t rd = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t funct3 = 0;
    uint8_t funct7 = 0;
    int32_t imm = 0;

    bool is_ecall() const
    {
        // RV32 SYSTEM opcode with funct3 == 0 indicates ECALL
        return opcode == 0x73 && funct3 == 0;
    }
};

// Decode a raw 32-bit instruction into fields.
// Kept here so ISA decoding is separated from CpuCore fetch logic.
inline DecodedInstruction decode_instruction(uint32_t raw)
{
    DecodedInstruction d{};
    d.raw = raw;

    d.opcode = raw & 0x7F;
    d.rd = (raw >> 7) & 0x1F;
    d.funct3 = (raw >> 12) & 0x07;
    d.rs1 = (raw >> 15) & 0x1F;
    d.rs2 = (raw >> 20) & 0x1F;
    d.funct7 = (raw >> 25) & 0x7F;

    switch (d.opcode)
    {
    case 0x13:
    case 0x03:
    case 0x67:
    case 0x73:
        d.imm = (int32_t)raw >> 20;
        break;

    case 0x23:
        d.imm = ((raw >> 7) & 0x1F) | (((int32_t)raw >> 25) << 5);
        break;

    case 0x63:
    {
        int32_t imm =
            (((raw >> 31) & 0x1) << 12) |
            (((raw >> 7) & 0x1) << 11) |
            (((raw >> 25) & 0x3F) << 5) |
            (((raw >> 8) & 0xF) << 1);
        d.imm = (imm << 19) >> 19; // sign extend 13-bit
        break;
    }

    case 0x17:
    case 0x37:
        d.imm = raw & 0xFFFFF000;
        break;

    case 0x6F:
        d.imm = (((raw >> 21) & 0x3FF) << 1) |
                (((raw >> 20) & 1) << 11) |
                (((raw >> 12) & 0xFF) << 12) |
                (((int32_t)raw >> 31) << 20);
        break;

    default:
        d.imm = 0;
        break;
    }

    return d;
}
