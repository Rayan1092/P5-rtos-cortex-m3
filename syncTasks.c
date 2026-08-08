#include "defs.h"

volatile unsigned long uartMutex = 1;

void mutexTake(volatile unsigned long *mutex)
{

    unsigned long failed = 1;
    do
    {
        unsigned long mval;

        asm volatile(
            "ldrex %0, [%1]"
            : "=r"(mval)
            : "r"(mutex)
            : "memory");

        if (!mval)
        {
            asm volatile("clrex" ::: "memory");
            continue;
        }

        asm volatile(
            "strex %0, %1, [%2]"
            : "=&r"(failed)
            : "r"(0UL), "r"(mutex)
            : "memory");

    } while (failed);
}

void mutexReturn(volatile unsigned long *mutex)
{
    asm volatile("dmb" ::: "memory");

    *mutex = 1;
}
