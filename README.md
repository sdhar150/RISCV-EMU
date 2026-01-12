# RV32IM User-Mode Emulator

A **RISC-V RV32IM user-mode emulator** capable of running **real ELF binaries** linked against **newlib**.  
It provides a minimal Linux-like process environment — including heap, stack, and syscalls — while executing instructions with architectural correctness.

---

## Key features

### CPU
- RV32I base ISA  
- M extension (multiply / divide)  
- Little-endian  
- Precise traps and ECALL handling  

### ELF loader
- Loads ELF32 EXEC binaries at their linked virtual addresses  
- Applies RISC-V relocations:
  - `R_RISCV_RELATIVE`
  - `R_RISCV_32`
  - `R_RISCV_JUMP_SLOT`  
- Correctly initializes `.text`, `.data`, `.bss`, and globals  

### Process model
- Flat virtual address space  
- `brk()`-based heap  
- `mmap()`-based anonymous mappings  
- Downward-growing stack  

### Syscalls
Implemented Linux-compatible calls:
- `read`
- `write`
- `brk`
- `mmap`
- `munmap`
- `exit`

This is sufficient to run **newlib** with:
- `printf`
- `malloc` / `free`
- `FILE`
- stdio buffering
- errno

---

## Memory layout

```
0x00000000  text + rodata
0x00400000  data + bss
           ↑ brk() grows heap
           free
           ↓ mmap()
           ↓
        stack (grows downward)
```

---

## Building

```
make
```

or:

```
g++ -std=c++17 -O2 -Iinclude \
    src/emulator/main.cpp \
    src/platform/ElfLoader.cpp \
    -o bin/emulator
```

---

## Running programs

```
bin/emulator demo/hello/hello.elf
bin/emulator demo/stdlib/stdlib_test.elf
bin/emulator demo/io/cat.elf
bin/emulator demo/rpn/rpn.elf
bin/emulator demo/stress/alloc.elf
```

---

## Testing

```
make test
```

This runs real ELF programs and compares their output against golden files in `tests/`.

---

## What this emulator is

This project implements a **real RISC-V user-mode runtime** comparable to QEMU user-mode or Spike + proxy kernel, but intentionally minimal and readable.

---

## What is intentionally out of scope

- Floating point (F/D)  
- Atomics (A)  
- Compressed instructions (C)  
- Virtual memory  
- Threads or signals  
- Dynamic loader (`ld.so`)  
- PIE executables  

---

## Documentation

See `docs/architecture.md` and `CONTRIBUTING.md`.

---

## Why this exists

Built to demonstrate **full-stack understanding of CPU architecture, ELF, ABI, memory management, and libc integration** in a compact, inspectable codebase.
