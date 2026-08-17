#include "defs.h"
#define HIGHEST_PRIORITY 0

void yeild(void)
{
    ICSR |= PENDSVSET;
}

void task1Handle(void)
{
    int counter = 0;
    int taken = 0;
    while (1)
    {
        if (!taken)
            mutexTake(&uartMutex);

        taken = 1;
        sendChar('A');
        if (counter >= 100)
            mutexReturn(&uartMutex);
        counter++;
    }
}

void task2Handle(void)
{
    int counter = 0;
    while (1)
    {
        if (counter >= 100)
        {
            runningTask->state = BLOCKEDT;
        }
        sendChar('B');
        counter++;
    }
}

void task3Handle(void)
{
    int counter = 0;
    int taken = 0;
    while (1)
    {
        if (!taken)
            mutexTake(&uartMutex);
        taken = 1;

        sendChar('C');

        if (counter >= 100)
            mutexReturn(&uartMutex);
        counter++;
    }
}

void task4Handle(void)
{
    while (1)
    {
        GPIOCODR ^= (1 << 13);

        for (volatile unsigned long i = 0; i < 1000000; i++)
        {
        }
    }
}

void nextTask(void)
{
    int numRuns = 0;
    unsigned long highestP = 100;
    int highestPIndex = -1;

    if (runningTask->state == RUNNINGT)
        runningTask->state = READYT;

    while (numRuns < NUMTASKS)
    {

        if (taskIndex > NUMTASKS - 1)
            taskIndex = 0;

        if (taskarr[taskIndex].epriority < highestP && taskarr[taskIndex].state == READYT)
        {
            highestPIndex = taskIndex;
            highestP = taskarr[taskIndex].epriority;

            if (taskarr[taskIndex].epriority == HIGHEST_PRIORITY)
            {
                runningTask = &taskarr[taskIndex];
                taskarr[taskIndex].state = RUNNINGT;
                taskIndex++;
                return;
            }
        }

        taskIndex++;
        numRuns++;
    }

    if (highestPIndex != -1)
    {
        runningTask = &taskarr[highestPIndex];
        taskarr[highestPIndex].state = RUNNINGT;
        taskIndex = highestPIndex + 1;
        return;
    }

    displayLine();
    displayLabel("ERROR: Deadlock Detected");

    while (1)
    {
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
