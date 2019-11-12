#include "syscall.h"

int A[1024];

int main()
{
    int i;
    for (i = 0; i < 1024; i *= 100)
    {
        A[i] = i;
    }

    Halt();
    /* not reached */
}