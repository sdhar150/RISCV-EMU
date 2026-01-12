#pragma once

#include <iostream>
#include "riscv/core/Trap.hpp"

// ------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------

template <size_t XLEN>
MemorySubsystem<XLEN>::MemorySubsystem(const MemoryMap &map)
{
    for (const auto &desc : map.regions)
    {
        MemoryRegion r;
        r.base = desc.base;
        r.size = desc.size;
        r.type = desc.type;

        if (desc.type == MemoryRegionType::RAM)
            r.data = new uint8_t[desc.size]();
        else
            r.data = nullptr;

        regions.push_back(r);
    }
}

template <size_t XLEN>
MemorySubsystem<XLEN>::~MemorySubsystem()
{
    for (auto &r : regions)
        delete[] r.data;
}

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

// MMIO handler for platform devices (UART, etc)
template <size_t XLEN>
bool MemorySubsystem<XLEN>::handle_mmio_write(AddrType addr, uint8_t value)
{
    // UART at 0x10000000
    if (addr == 0x10000000)
    {
        std::cout << static_cast<char>(value);
        std::cout.flush();
        return true;
    }
    return false;
}

// Locate the region covering [addr, addr+size).
// Returns nullptr if no region maps this address range.
template <size_t XLEN>
MemoryRegion *MemorySubsystem<XLEN>::find_region(AddrType addr, size_t size)
{
    for (auto &r : regions)
    {
        if (addr >= r.base &&
            addr + size <= r.base + r.size)
            return &r;
    }
    return nullptr;
}

template <size_t XLEN>
bool MemorySubsystem<XLEN>::is_mapped(AddrType addr, size_t size) const
{
    for (const auto &r : regions)
    {
        if (addr >= r.base &&
            addr + size <= r.base + r.size)
            return true;
    }
    return false;
}

// ------------------------------------------------------------
// Loads
// ------------------------------------------------------------

template <size_t XLEN>
uint8_t MemorySubsystem<XLEN>::read_byte(AddrType addr)
{
    MemoryRegion *r = find_region(addr, 1);
    if (!r)
        throw Trap{TrapCause::LoadAccessFault, 0, (uint32_t)addr, 0};

    if (r->type == MemoryRegionType::MMIO)
        return 0;

    return r->data[addr - r->base];
}

template <size_t XLEN>
uint32_t MemorySubsystem<XLEN>::read_half(AddrType addr)
{
    if (addr & 1)
        throw Trap{TrapCause::MisalignedAccess, 0, (uint32_t)addr, 0};

    MemoryRegion *r = find_region(addr, 2);
    if (!r)
        throw Trap{TrapCause::LoadAccessFault, 0, (uint32_t)addr, 0};

    uint32_t off = addr - r->base;
    return r->data[off] | (r->data[off + 1] << 8);
}

template <size_t XLEN>
uint32_t MemorySubsystem<XLEN>::read_word(AddrType addr)
{
    if (addr & 3)
        throw Trap{TrapCause::MisalignedAccess, 0, (uint32_t)addr, 0};

    MemoryRegion *r = find_region(addr, 4);
    if (!r)
        throw Trap{TrapCause::LoadAccessFault, 0, (uint32_t)addr, 0};

    uint32_t off = addr - r->base;
    return r->data[off] |
           (r->data[off + 1] << 8) |
           (r->data[off + 2] << 16) |
           (r->data[off + 3] << 24);
}

// ------------------------------------------------------------
// Stores
// ------------------------------------------------------------

template <size_t XLEN>
bool MemorySubsystem<XLEN>::write_byte(AddrType addr, uint8_t value)
{
    MemoryRegion *r = find_region(addr, 1);
    if (!r)
        throw Trap{TrapCause::StoreAccessFault, 0, (uint32_t)addr, 0};

    if (r->type == MemoryRegionType::MMIO)
    {
        return handle_mmio_write(addr, value);
    }

    r->data[addr - r->base] = value;
    return true;
}

template <size_t XLEN>
bool MemorySubsystem<XLEN>::write_half(AddrType addr, uint16_t value)
{
    if (addr & 1)
        throw Trap{TrapCause::MisalignedAccess, 0, (uint32_t)addr, 0};

    MemoryRegion *r = find_region(addr, 2);
    if (!r)
        throw Trap{TrapCause::StoreAccessFault, 0, (uint32_t)addr, 0};

    uint32_t off = addr - r->base;
    r->data[off] = value & 0xFF;
    r->data[off + 1] = (value >> 8) & 0xFF;
    return true;
}

template <size_t XLEN>
bool MemorySubsystem<XLEN>::write_word(AddrType addr, uint32_t value)
{
    if (addr & 3)
        throw Trap{TrapCause::MisalignedAccess, 0, (uint32_t)addr, 0};

    MemoryRegion *r = find_region(addr, 4);
    if (!r)
        throw Trap{TrapCause::StoreAccessFault, 0, (uint32_t)addr, 0};

    uint32_t off = addr - r->base;
    r->data[off] = value & 0xFF;
    r->data[off + 1] = (value >> 8) & 0xFF;
    r->data[off + 2] = (value >> 16) & 0xFF;
    r->data[off + 3] = (value >> 24) & 0xFF;
    return true;
}

template <size_t XLEN>
void MemorySubsystem<XLEN>::memset(AddrType addr, uint8_t value, size_t size)
{
    for (size_t i = 0; i < size; i++)
        write_byte(addr + i, value);
}
