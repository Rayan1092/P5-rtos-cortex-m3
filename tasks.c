#include "defs.h"

#define STATUS_OFFSET 0x4
#define STATUS_MASK (1 << 0)

unsigned long done1 = 0;
unsigned long done2 = 0;

void sendChar(char c)
{
    unsigned long *data = (unsigned long *)UARTBASE;
    unsigned long *status = (unsigned long *)(UARTBASE + STATUS_OFFSET);

    while (*status & STATUS_MASK)
    {
    }

    *data = c;
}

void yeild(void)
{
    ICSR |= PENDSVSET;
}

void task1Handle(void)
{
    void *test1 = myMalloc(40);
    readHeaders();
    myFree(test1);
    done1 = 1;

    while (1)
    {
    }
}

void task2Handle(void)
{
    void *test2 = myMalloc(40);
    readHeaders();
    myFree(test2);
    done2 = 1;

    while (1)
    {
    }
}

void task3Handle(void)
{
    while (!(done1 && done2))
    {
    }
    readHeaders();
    while (1)
    {
    }
}

void nextTask(void)
{
    int numRuns = 0;

    if (runningTask->state == RUNNINGT)
        runningTask->state = READYT;

    while (1)
    {
        if (taskIndex > NUMTASKS - 1)
            taskIndex = 0;

        if (taskarr[taskIndex].state == READYT)
        {
            runningTask = &taskarr[taskIndex];
            runningTask->state = RUNNINGT;
            taskIndex++;
            break;
        }
        taskIndex++;
        numRuns++;

        if (numRuns == NUMTASKS)
        {
            displayLabel("ERROR: Deadlock Detected");

            while (1)
            {
            }
        }
    }
}

void taskInit(struct Task *task)
{
    task->state = READYT;

    unsigned long *stackTop = &task->stack[63];

    *stackTop = 0x01000000;
    stackTop--;

    *stackTop = ((unsigned long)task->taskHandle) & PC_MASK;
    stackTop--;

    for (int i = 0; i < 15; i++)
    {
        if (i == 6)
            *stackTop = 0xFFFFFFF9;
        else
            *stackTop = 0;

        if (i != 14)
            stackTop--;
    }

    task->sp = stackTop;
}

__attribute__((naked)) void pendSVHandle(void)
{
    asm volatile(
        "clrex \n"
        "push {r4-r11, lr} \n"
        "ldr r0, =runningTask \n"
        "ldr r0, [r0] \n"
        "str sp, [r0] \n"

        "bl nextTask \n"

        "ldr r0, =runningTask \n"
        "ldr r0, [r0] \n"
        "ldr sp, [r0] \n"
        "pop {r4-r11, lr} \n"

        "bx lr \n" ::: "memory");
}

void sysTickHandler(void)
{
    ICSR |= PENDSVSET;
}
