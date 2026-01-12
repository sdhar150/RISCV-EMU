int main(void)
{
    const char *msg = "Hello from C with proper crt0!\n";
    const int len = sizeof("Hello from C with proper crt0!\n") - 1;

    asm volatile(
        "li a0, 1\n"
        "mv a1, %0\n"
        "mv a2, %1\n"
        "li a7, 64\n"
        "ecall\n"
        :
        : "r"(msg), "r"(len)
        : "a0", "a1", "a2", "a7");

    return 0;
}
