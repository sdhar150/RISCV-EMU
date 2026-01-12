#include "riscv/platform/ElfLoader.hpp"
#include <elf.h>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <algorithm>

#define R_RISCV_32 1
#define R_RISCV_RELATIVE 3
#define R_RISCV_JUMP_SLOT 5

// End of loaded ELF image (used by brk())
uint32_t g_image_end = 0;

bool ElfLoader::load(const std::string &path,
                     MemorySubsystem<32> &memory,
                     ArchitecturalState<32> &state)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open ELF");

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    file.read((char *)data.data(), size);

    const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)data.data();

    if (memcmp(ehdr->e_ident, ELFMAG, 4))
        throw std::runtime_error("Not ELF");

    if (ehdr->e_machine != EM_RISCV)
        throw std::runtime_error("Not RISC-V");

    if ((ehdr->e_flags & EF_RISCV_FLOAT_ABI_SOFT) != EF_RISCV_FLOAT_ABI_SOFT)
        throw std::runtime_error("FPU not supported");

    // ---------------- Load PT_LOAD segments ----------------
    const Elf32_Phdr *phdrs =
        (const Elf32_Phdr *)(data.data() + ehdr->e_phoff);

    g_image_end = 0;

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        const Elf32_Phdr &ph = phdrs[i];
        if (ph.p_type != PT_LOAD)
            continue;

        uint32_t vaddr = ph.p_vaddr;
        uint32_t off = ph.p_offset;
        uint32_t filesz = ph.p_filesz;
        uint32_t memsz = ph.p_memsz;

        for (uint32_t j = 0; j < filesz; j++)
            memory.write_byte(vaddr + j, data[off + j]);

        for (uint32_t j = filesz; j < memsz; j++)
            memory.write_byte(vaddr + j, 0);

        g_image_end = std::max(g_image_end, vaddr + memsz);
    }

    // ---------------- Apply relocations ----------------
    const Elf32_Shdr *shdrs =
        (const Elf32_Shdr *)(data.data() + ehdr->e_shoff);

    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        const Elf32_Shdr &sh = shdrs[i];
        if (sh.sh_type != SHT_RELA)
            continue;

        const Elf32_Rela *relas =
            (const Elf32_Rela *)(data.data() + sh.sh_offset);

        const Elf32_Shdr &symhdr = shdrs[sh.sh_link];
        const Elf32_Sym *syms =
            (const Elf32_Sym *)(data.data() + symhdr.sh_offset);

        size_t count = sh.sh_size / sizeof(Elf32_Rela);

        for (size_t j = 0; j < count; j++)
        {
            const Elf32_Rela &r = relas[j];

            uint32_t type = ELF32_R_TYPE(r.r_info);
            uint32_t sym = ELF32_R_SYM(r.r_info);
            uint32_t addr = r.r_offset;
            int32_t A = r.r_addend;
            uint32_t S = sym ? syms[sym].st_value : 0;

            uint32_t result;

            switch (type)
            {
            case R_RISCV_RELATIVE:
                result = A;
                break;

            case R_RISCV_32:
            case R_RISCV_JUMP_SLOT:
                result = S + A;
                break;

            default:
                throw std::runtime_error("Unsupported RISC-V relocation");
            }

            for (int k = 0; k < 4; k++)
                memory.write_byte(addr + k, (result >> (8 * k)) & 0xFF);
        }
    }

    state.set_pc(ehdr->e_entry);
    return true;
}
