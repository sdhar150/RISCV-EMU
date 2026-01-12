#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    printf("printf works %d\n", 123);

    int *p = malloc(4 * sizeof(int));
    p[0] = 456;
    printf("malloc works %d\n", p[0]);

    return 0;
}
