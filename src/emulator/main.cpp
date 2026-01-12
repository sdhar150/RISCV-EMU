#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>

#include "riscv/core/Processor.hpp"
#include "riscv/memory/Memory.hpp"
#include "riscv/platform/ElfLoader.hpp"

int main(int argc, char **argv)
{
    bool trace = false;
    const char *trace_path = "trace.log";
    const char *elf = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--trace"))
            trace = true;
        else if (!strcmp(argv[i], "--trace-file") && i + 1 < argc)
            trace_path = argv[++i];
        if (!strcmp(argv[i], "--version"))
        {
            std::cout << "rv32im-emulator 1.0 (RV32IM user-mode)\n";
            return 0;
        }
        if (!strcmp(argv[i], "--help"))
        {
            std::cout << "Usage: emulator [--trace] [--trace-file file] program.elf\n"
                         "RV32IM user-mode emulator\n";
            return 0;
        }

        else
            elf = argv[i];
    }

    if (!elf)
    {
        std::cerr << "Usage: emulator [--trace] [--trace-file file] program.elf\n";
        return 1;
    }

    MemoryMap map{
        {
            {0x00000000, 4 * 1024 * 1024, MemoryRegionType::RAM},   // program
            {0x00400000, 124 * 1024 * 1024, MemoryRegionType::RAM}, // stack + heap
            {0x10000000, 0x1000, MemoryRegionType::MMIO},           // UART
        }};

    MemorySubsystem<32> memory(map);
    ArchitecturalState<32> state;
    CpuCore<32> cpu(state, memory);

    std::ofstream trace_file;
    if (trace)
    {
        trace_file.open(trace_path);
        cpu.set_trace(true);
        cpu.set_trace_stream(&trace_file);
    }

    ElfLoader::load(elf, memory, state);

    auto start = std::chrono::high_resolution_clock::now();

    while (cpu.step())
    {
    }

    auto end = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();

    uint64_t insts = cpu.get_inst_count();

    std::cerr << "\n--- Emulator stats ---\n";
    std::cerr << "Instructions: " << insts << "\n";
    std::cerr << "Time: " << seconds << " s\n";
    if (seconds > 0)
        std::cerr << "IPS: " << (insts / seconds) << "\n";

    return 0;
}
