#include "defs.h"

#define PC_MASK 0xFFFFFFFE
#define SHPR3 (*(volatile unsigned long *)0xE000ED20)

// pendsv low priority
#define PENDSVLOWP (0xFF << 16)
#define CTRL_OFFSET 0x8

// specific to systick (control status reg)
#define CSR (*(unsigned long *)0xE000E010)
#define RVR (*(unsigned long *)0xE000E014)
#define CVR (*(unsigned long *)0xE000E018)
// bit 0 en, bit 1 send interupt, bit 2 use processor clock
#define SYSTICKSET (0x7 << 0)

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

void sysTickInit(void)
{
    // runs at 25Mhz, 1ms switch hence 25000000 / 1000 = 25000 - 1 (at 0)
    RVR = 24999;
    CVR = 0;
    CSR |= SYSTICKSET;
}

void initializer(void)
{
    runningTask = &task1;

    volatile unsigned long *ctrl = (unsigned long *)(UARTBASE + CTRL_OFFSET);
    *ctrl |= (1 << 0);

    SHPR3 |= PENDSVLOWP;

    taskInit(&task1, task1Handle);
    taskInit(&task2, task2Handle);

    sysTickInit();

    main_to_task(task1Handle);
}