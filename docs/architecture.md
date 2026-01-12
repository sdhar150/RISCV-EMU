# RV32IM User-Mode Emulator Architecture

## Overview
This project implements a **RISC-V RV32IM user-mode emulator** capable of running real ELF binaries linked against **newlib**. It provides a Linux-like process model (heap, stack, and I/O) while executing instructions with architectural correctness.

---

## Supported ISA

| Feature | Status |
|--------|--------|
| RV32I  | ✔ |
| M (mul/div) | ✔ |
| F / D | ✘ |
| A (atomics) | ✘ |
| C (compressed) | ✘ |
| Endianness | Little |
| ABI | ILP32, soft-float |

---

## Executable Format

| Feature | Status |
|--------|--------|
| ELF32 EXEC | ✔ |
| ELF relocations | ✔ (RELATIVE, 32, JUMP_SLOT) |
| PIE | ✘ |
| Dynamic loader | ✘ |

The emulator loads ELF executables at their linked virtual addresses and applies static relocations.

---

## Memory Model

```
0x00000000  text + rodata
0x00400000  data + bss
           ↑ brk() grows heap
           free
           ↓ mmap()
           ↓
        stack (grows downward)
```

Memory is divided into:
- RAM regions
- MMIO (UART)

All accesses are bounds-checked.

---

## Syscalls

| Number | Name |
|--------|------|
| 63 | read |
| 64 | write |
| 80, 214 | brk |
| 222 | mmap |
| 215 | munmap |
| 93 | exit |

The heap is managed using `brk()` and `mmap()` in a Linux-compatible layout sufficient for newlib malloc.

---

## CPU Model

- All instructions are decoded once into a `DecodedInstruction`
- The execution engine applies architectural semantics
- PC is updated exactly once per instruction
- ECALL is implemented as a precise trap

This allows:
- correct exception behavior
- restartable syscalls
- deterministic execution

---

## What This Emulator Is

This emulator is a **real RISC-V user-mode runtime** capable of executing compiler-generated binaries using a real C standard library.

It is comparable in scope to:
- QEMU user-mode
- Spike + proxy kernel

but intentionally minimal and readable.

