#include "defs.h"

#define PC_MASK 0xFFFFFFFE
#define SHPR3 (*(volatile unsigned long *)0xE000ED20)

// pendsv low priority
#define PENDSVLOWP (0xFF << 16)
#define CTRL_OFFSET 0x8

void taskInit(struct Task *task, void (*taskHandle)(void))
{
    unsigned long *stackTop = &task->stack[63];

    *stackTop = 0x01000000;
    stackTop--;

    *stackTop = ((unsigned long)taskHandle) & PC_MASK;
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

__attribute__((naked)) void main_to_task(void (*taskH)(void))
{
    asm volatile(
        "ldr r1, =runningTask \n"
        "ldr r1, [r1] \n"
        "ldr sp, [r1] \n"

        "mov pc, r0 \n");
}

void initializer(void)
{
    runningTask = &task1;

    volatile unsigned long *ctrl = (unsigned long *)(UARTBASE + CTRL_OFFSET);
    *ctrl |= (1 << 0);

    SHPR3 |= PENDSVLOWP;

    taskInit(&task1, task1Handle);
    taskInit(&task2, task2Handle);

    main_to_task(task1Handle);
}