# Contributing

This emulator intentionally supports only:

- RV32I + M
- ELF32 EXEC
- newlib user-mode ABI

Out of scope:
- Floating point (F/D)
- Atomics (A)
- Compressed instructions (C)
- Virtual memory
- Threads, signals, or PIE

All changes must:
- Preserve deterministic execution
- Pass `make test`
- Maintain architectural correctness
