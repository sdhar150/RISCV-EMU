#pragma once
#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include <sstream>

constexpr std::size_t N_GEN_PURPOSE_REGS = 32;

// Architectural State: registers, PC, future CSRs
template <std::size_t XLEN>
struct ArchitecturalState
{
    static_assert(XLEN == 32 || XLEN == 64, "XLEN must be 32 or 64");

    using RegType = std::conditional_t<XLEN == 32, uint32_t, uint64_t>;

    RegType x[N_GEN_PURPOSE_REGS]{};
    RegType pc{};

    // ===== Register Access =====
    RegType reg(std::size_t i) const
    {
        if (i >= N_GEN_PURPOSE_REGS)
            throw std::out_of_range("Invalid register index");
        return (i == 0) ? 0 : x[i];
    }

    void set_reg(std::size_t i, RegType value)
    {
        if (i >= N_GEN_PURPOSE_REGS)
            throw std::out_of_range("Invalid register index");
        if (i != 0)
            x[i] = value; // x0 is hardwired to zero
    }

    // ===== PC Control =====
    RegType get_pc() const
    {
        return pc;
    }

    // Only the execution engine should update PC.
    // CpuCore must never modify PC directly.
    void set_pc(RegType value)
    {
        pc = value;
    }

    // ===== Trap Handling =====
    [[noreturn]] void raise_trap() const
    {
        std::ostringstream oss;
        oss << "CPU trap PC=0x" << std::hex << get_pc();
        throw std::runtime_error(oss.str());
    }

    // ===== Reset CPU State =====
    void reset(RegType pc_start = 0)
    {
        // Clear all general-purpose registers
        for (std::size_t i = 0; i < N_GEN_PURPOSE_REGS; ++i)
            x[i] = 0;

        // Set program counter
        pc = pc_start;
    }
    uint32_t symbol(const std::string &);
};
