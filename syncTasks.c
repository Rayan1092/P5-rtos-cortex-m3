#include "defs.h"

volatile unsigned long uartMutex = 1;
volatile unsigned long heapMutex = 1;

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
            disableInterrupts();

            runningTask->awaitingMutex = mutex;
            runningTask->state = BLOCKEDT;
            yeild();

            enableInterrupts();
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
    disableInterrupts();

    for (int i = 0; i < NUMTASKS; i++)
    {
        if (taskarr[i].state == BLOCKEDT && taskarr[i].awaitingMutex == mutex)
        {
            taskarr[i].state = READYT;
            taskarr[i].awaitingMutex = 0;
        }
    }

    *mutex = 1;

    enableInterrupts();
}
