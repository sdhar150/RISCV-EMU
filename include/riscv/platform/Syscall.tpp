#pragma once

#include <iostream>

extern uint32_t g_image_end;

// This implements a minimal Unix-like process memory model:
// - brk() grows a contiguous heap upward
// - mmap() allocates anonymous memory above the heap
// - stack grows downward and is protected from collision

template <typename State, typename Memory>
bool SyscallHandler<State, Memory>::handle(State &state)
{
    uint32_t syscall = state.reg(17); // a7

    uint32_t a0 = state.reg(10);
    uint32_t a1 = state.reg(11);
    uint32_t a2 = state.reg(12);

    switch (syscall)
    {
    case 63: // read(fd, buf, len)
    {
        if (a0 == 0) // stdin
        {
            int c = std::cin.get();
            if (c == EOF)
            {
                state.set_reg(10, 0);
                return true;
            }

            memory.write_byte(a1, (uint8_t)c);
            uint32_t i = 1;

            while (i < a2 && std::cin.rdbuf()->in_avail() > 0)
            {
                c = std::cin.get();
                if (c == EOF)
                    break;
                memory.write_byte(a1 + i, (uint8_t)c);
                i++;
            }

            state.set_reg(10, i);
            return true;
        }

        state.set_reg(10, static_cast<uint32_t>(-1));
        return true;
    }

    case 64: // write(fd, buf, len)
    {
        if (a0 == 1 || a0 == 2)
        {
            for (uint32_t i = 0; i < a2; ++i)
                std::cout << (char)memory.read_byte(a1 + i);
            std::cout.flush();
            state.set_reg(10, a2);
            return true;
        }
        state.set_reg(10, static_cast<uint32_t>(-1));
        return true;
    }

    case 80:  // newlib brk
    case 214: // Linux brk
    {
        uint32_t new_brk = a0;
        uint32_t sp = state.reg(2);

        // Lazy init
        if (program_break == 0)
        {
            program_break = g_image_end;
            mmap_top = sp - 0x10000;
        }

        if (new_brk == 0)
        {
            state.set_reg(10, program_break);
            return true;
        }

        if (new_brk >= mmap_top - 4096 || new_brk < program_break)
        {
            state.set_reg(10, (uint32_t)-1);
            return true;
        }

        if (new_brk > program_break)
            memory.memset(program_break, 0, new_brk - program_break);

        program_break = new_brk;
        state.set_reg(10, program_break);
        return true;
    }

    case 222: // mmap
    {
        uint32_t size = (a1 + 4095) & ~4095;
        uint32_t sp = state.reg(2);

        if (program_break == 0)
        {
            program_break = g_image_end;
            mmap_top = sp - 0x10000;
        }

        uint32_t addr = mmap_top - size;

        if (addr <= program_break + 4096)
        {
            state.set_reg(10, (uint32_t)-1);
            return true;
        }

        mmap_top = addr;
        memory.memset(addr, 0, size);
        state.set_reg(10, addr);
        return true;
    }

    case 215: // munmap
    {
        uint32_t addr = a0;
        uint32_t size = (a1 + 4095) & ~4095;

        if (addr == mmap_top)
            mmap_top += size;

        state.set_reg(10, 0);
        return true;
    }

    case 93: // exit
    {
        std::cout << "\n[program exited with code " << a0 << "]\n";
        return false;
    }

    default:
        std::cerr << "Unknown syscall " << syscall << "\n";
        return false;
    }
}
