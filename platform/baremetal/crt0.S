    .section .text.start
    .globl _start
    .globl _exit

_start:
    /* Set up stack pointer */
    la sp, __stack_top

    /* Clear BSS */
    la t0, __bss_start
    la t1, __bss_end
1:
    bge t0, t1, 2f
    sw  zero, 0(t0)
    addi t0, t0, 4
    j   1b
2:

    /* Call main() */
    call main

    /* Exit with return value */
    mv a0, a0
    li a7, 93          /* exit syscall */
    ecall

_exit:
    j _exit            /* hang if exit returns */
