#include <unistd.h>

int main()
{
    char buf[128];
    int n;

    while ((n = read(0, buf, sizeof(buf))) > 0)
        write(1, buf, n);

    return 0;
}
