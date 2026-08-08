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
            asm volatile(
                // ddisabling all interupts
                "mov r0, #1 \n"
                "msr primask, r0 \n"
                "clrex \n" ::: "memory", "r0");

            runningTask->awaitingMutex = mutex;
            runningTask->state = BLOCKEDT;
            ICSR |= PENDSVSET;

            asm volatile(
                "mov r0, #0 \n"
                "msr primask, r0 \n" ::: "memory", "r0");
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
    asm volatile(
        "dmb \n"
        "mov r0, #1 \n"
        "msr primask, r0 \n"
        :
        :
        : "memory", "r0");

    for (int i = 0; i < NUMTASKS; i++)
    {
        if (taskarr[i].state == BLOCKEDT && taskarr[i].awaitingMutex == mutex)
        {
            taskarr[i].state = READYT;
            taskarr[i].awaitingMutex = 0;
        }
    }

    *mutex = 1;

    asm volatile(
        "mov r0, #0 \n"
        "msr primask, r0 \n" ::: "memory", "r0");
}
