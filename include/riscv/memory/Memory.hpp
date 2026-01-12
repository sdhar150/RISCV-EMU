#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <type_traits>

#include "riscv/core/Trap.hpp"

// ============================================================
// MemorySubsystem
//
// Models a process virtual address space composed of regions
// (RAM or MMIO). All memory accesses go through this layer.
//
// It enforces:
//   - address translation
//   - region boundaries
//   - alignment rules
//   - device I/O dispatch
//
// This mirrors how a real OS kernel mediates memory access.
// ============================================================
enum class MemoryRegionType
{
    RAM,
    MMIO
};

// MemoryRegionDesc describes a region in the virtual address space
// provided by the platform (RAM or MMIO).
struct MemoryRegionDesc
{
    uint32_t base;
    uint32_t size;
    MemoryRegionType type;
};

// ------------------------------------------------------------
// Internal memory region (backed storage)
// ------------------------------------------------------------

struct MemoryRegion
{
    uint32_t base;
    uint32_t size;
    MemoryRegionType type;
    uint8_t *data; // nullptr for MMIO
};

// ------------------------------------------------------------
// Memory map (frontend-owned)
// ------------------------------------------------------------

struct MemoryMap
{
    std::vector<MemoryRegionDesc> regions;
};

// ------------------------------------------------------------
// Memory subsystem (region-based, map-driven)
// ------------------------------------------------------------

template <size_t XLEN>
class MemorySubsystem
{
    static_assert(XLEN == 32 || XLEN == 64, "XLEN must be 32 or 64");

  public:
    using AddrType = std::conditional_t<XLEN == 32, uint32_t, uint64_t>;

    explicit MemorySubsystem(const MemoryMap &map);
    ~MemorySubsystem();

    MemorySubsystem(const MemorySubsystem &) = delete;
    MemorySubsystem &operator=(const MemorySubsystem &) = delete;

    // Loads
    uint8_t read_byte(AddrType addr);
    uint32_t read_half(AddrType addr);
    uint32_t read_word(AddrType addr);

    // Stores
    bool write_byte(AddrType addr, uint8_t value);
    bool write_half(AddrType addr, uint16_t value);
    bool write_word(AddrType addr, uint32_t value);

    bool is_mapped(AddrType addr, size_t size) const;
    void memset(AddrType addr, uint8_t value, size_t size);

  private:
    std::vector<MemoryRegion> regions;
    bool handle_mmio_write(AddrType addr, uint8_t value);
    MemoryRegion *find_region(AddrType addr, size_t size);
};

#include "Memory.tpp"
