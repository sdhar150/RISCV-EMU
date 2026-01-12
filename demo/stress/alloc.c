#include <stdlib.h>
#include <stdio.h>

int main()
{
    for (int i = 0; i < 1000; i++)
    {
        int *p = malloc(1024);
        if (!p)
        {
            printf("alloc failed\n");
            return 1;
        }
        p[0] = i;
        free(p);
    }
    printf("allocator ok\n");
    return 0;
}
