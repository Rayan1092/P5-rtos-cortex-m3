#include "defs.h"

volatile struct Mutex uartMutex = {1, 0};
volatile struct Mutex heapMutex = {1, 0};

void mutexTake(volatile struct Mutex *mutex)
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

            if (mutex->holder != 0)
                if (mutex->holder->epriority > runningTask->epriority)
                    mutex->holder->epriority = runningTask->epriority;

            yeild();
            enableInterrupts();
            continue;
        }

        asm volatile(
            "strex %0, %1, [%2]"
            : "=&r"(failed)
            : "r"(0UL), "r"(mutex)
            : "memory");

        if (!failed)
        {
            mutex->holder = runningTask;
            displayLabel("Mutex Taken");
        }

    } while (failed);
}

void mutexReturn(volatile struct Mutex *mutex)
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

    mutex->val = 1;
    mutex->holder = 0;
    runningTask->epriority = runningTask->ogpriority;

    enableInterrupts();
}
