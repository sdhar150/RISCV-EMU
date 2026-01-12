#define SYS_read 63
#define SYS_write 64

static inline int sys_read(int fd, void *buf, int len)
{
    int ret;
    asm volatile(
        "mv a0, %1\n"
        "mv a1, %2\n"
        "mv a2, %3\n"
        "li a7, %4\n"
        "ecall\n"
        "mv %0, a0\n"
        : "=r"(ret)
        : "r"(fd), "r"(buf), "r"(len), "i"(SYS_read)
        : "a0", "a1", "a2", "a7");
    return ret;
}

static inline int sys_write(int fd, const void *buf, int len)
{
    int ret;
    asm volatile(
        "mv a0, %1\n"
        "mv a1, %2\n"
        "mv a2, %3\n"
        "li a7, %4\n"
        "ecall\n"
        "mv %0, a0\n"
        : "=r"(ret)
        : "r"(fd), "r"(buf), "r"(len), "i"(SYS_write)
        : "a0", "a1", "a2", "a7");
    return ret;
}

/* ================= calculator ================= */

#define STACK_SIZE 128
int stack[STACK_SIZE];
int sp = 0;

void push(int v)
{
    stack[sp++] = v;
}
int pop()
{
    return stack[--sp];
}
int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int parse_int(const char *s)
{
    int v = 0;
    while (*s)
        v = v * 10 + (*s++ - '0');
    return v;
}

int main()
{
    char buf[64];
    int n;

    while ((n = sys_read(0, buf, sizeof(buf) - 1)) > 0)
    {
        buf[n] = 0;

        for (int i = 0; buf[i]; ++i)
        {
            if (buf[i] == ' ' || buf[i] == '\n')
                continue;

            if (is_digit(buf[i]))
            {
                char num[16];
                int j = 0;
                while (is_digit(buf[i]))
                    num[j++] = buf[i++];
                num[j] = 0;
                push(parse_int(num));
                i--;
                continue;
            }

            int b = pop();
            int a = pop();

            if (buf[i] == '+')
                push(a + b);
            else if (buf[i] == '-')
                push(a - b);
            else if (buf[i] == '*')
                push(a * b);
            else if (buf[i] == '/')
                push(a / b);
        }
    }

    int result = pop();
    char out[16];
    int len = 0;

    if (result == 0)
        out[len++] = '0';
    else
    {
        int r = result;
        char tmp[16];
        int t = 0;
        while (r)
        {
            tmp[t++] = '0' + (r % 10);
            r /= 10;
        }
        while (t--)
            out[len++] = tmp[t];
    }

    out[len++] = '\n';
    sys_write(1, out, len);
    return 0;
}
