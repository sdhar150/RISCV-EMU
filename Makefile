# ============================================================
# Project: RV32 Bare-Metal Emulator
# ============================================================

# ------------------------------------------------------------
# Host OS detection
# ------------------------------------------------------------
UNAME_S := $(shell uname -s)

ifeq ($(OS),Windows_NT)
    HOST_OS := windows
    EXE := .exe
    MKDIR := mkdir -p
    RM := rm -f
else ifeq ($(UNAME_S),Darwin)
    HOST_OS := macos
    EXE :=
    MKDIR := mkdir -p
    RM := rm -f
else
    HOST_OS := linux
    EXE :=
    MKDIR := mkdir -p
    RM := rm -f
endif

# ------------------------------------------------------------
# Host compiler (emulator)
# ------------------------------------------------------------
CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -O2 -g
INCLUDES  := -Iinclude

# ------------------------------------------------------------
# RISC-V toolchain (guest programs)
# ------------------------------------------------------------
RISCV_CC      := riscv64-elf-gcc
RISCV_CFLAGS := -march=rv32im -mabi=ilp32 -nostartfiles
RISCV_LIBS   := -static -lc -lgcc

# ------------------------------------------------------------
# Directories
# ------------------------------------------------------------
BIN_DIR      := bin
SRC_DIR      := src
PLATFORM_DIR := platform/baremetal
DEMO_DIR     := demo

# ------------------------------------------------------------
# Platform ABI
# ------------------------------------------------------------
LINKER_SCRIPT := $(PLATFORM_DIR)/linker.ld
CRT0          := $(PLATFORM_DIR)/crt0.S

# ------------------------------------------------------------
# Emulator
# ------------------------------------------------------------
EMULATOR := $(BIN_DIR)/emulator$(EXE)

EMULATOR_SRC := \
	$(SRC_DIR)/emulator/main.cpp \
	$(SRC_DIR)/platform/ElfLoader.cpp

# ------------------------------------------------------------
# Demo programs
# ------------------------------------------------------------
HELLO_SRC  := $(DEMO_DIR)/hello/hello.c
HELLO_ELF  := $(DEMO_DIR)/hello/hello.elf

STDLIB_SRC := $(DEMO_DIR)/stdlib/stdlib_test.c
STDLIB_ELF := $(DEMO_DIR)/stdlib/stdlib_test.elf

RPN_SRC    := $(DEMO_DIR)/rpn/rpn.c
RPN_ELF    := $(DEMO_DIR)/rpn/rpn.elf

CAT_SRC    := $(DEMO_DIR)/io/cat.c
CAT_ELF    := $(DEMO_DIR)/io/cat.elf

ALLOC_SRC  := $(DEMO_DIR)/stress/alloc.c
ALLOC_ELF  := $(DEMO_DIR)/stress/alloc.elf

DEMO_ELFS := \
	$(HELLO_ELF) \
	$(STDLIB_ELF) \
	$(RPN_ELF) \
	$(CAT_ELF) \
	$(ALLOC_ELF)

# ------------------------------------------------------------
# Phony targets
# ------------------------------------------------------------
.PHONY: all clean test emulator demos

# ============================================================
# Default target
# ============================================================

all: emulator demos

# ============================================================
# Build emulator
# ============================================================

$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)

emulator: $(EMULATOR)

$(EMULATOR): $(BIN_DIR) $(EMULATOR_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(EMULATOR_SRC) -o $@

# ============================================================
# Pattern rule for building demo ELFs
# ============================================================

%.elf: %.c $(CRT0) $(LINKER_SCRIPT)
	$(RISCV_CC) \
	  $(RISCV_CFLAGS) \
	  -Wl,-T,$(LINKER_SCRIPT) \
	  $(CRT0) \
	  $< \
	  $(RISCV_LIBS) \
	  -o $@

# ============================================================
# Build all demos
# ============================================================

demos: $(DEMO_ELFS)

# ============================================================
# Run demo suite
# ============================================================

test: emulator demos
	@echo "[hello]"
	./$(EMULATOR) $(HELLO_ELF) | grep -q "Hello"

	@echo "[stdlib]"
	./$(EMULATOR) $(STDLIB_ELF) | grep -q "malloc works"

	@echo "[rpn]"
	echo "3 4 +" | ./$(EMULATOR) $(RPN_ELF) | grep -q "7"

	@echo "[io]"
	echo "abc" | ./$(EMULATOR) $(CAT_ELF) | grep -q "abc"

	@echo "[stress]"
	./$(EMULATOR) $(ALLOC_ELF) | grep -q "allocator ok"

	@echo "All demos passed."

# ============================================================
# Cleanup
# ============================================================

clean:
	$(RM) $(EMULATOR)
	$(RM) $(DEMO_ELFS)
	$(RM) -r $(BIN_DIR)
