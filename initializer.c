#include "defs.h"

#define SHPR3 (*(volatile unsigned long *)0xE000ED20)
// pendsv low priority
#define PENDSVLOWP (0xFF << 16)

// specific to systick (control status reg)
#define CSR (*(volatile unsigned long *)0xE000E010)
#define RVR (*(volatile unsigned long *)0xE000E014)
#define CVR (*(volatile unsigned long *)0xE000E018)
// bit 0 en, bit 1 send interupt, bit 2 use processor clock
#define SYSTICKSET (0x7 << 0)

int taskIndex = 0;
struct Task taskarr[NUMTASKS];
struct Task *runningTask;

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

void pendSVInit(void)
{
    SHPR3 |= PENDSVLOWP;
}

void taskSetup(void)
{
    taskarr[0].taskHandle = task1Handle;
    taskarr[1].taskHandle = task2Handle;
    taskarr[2].taskHandle = task3Handle;

    for (int i = 0; i < NUMTASKS; i++)
    {
        taskInit(&taskarr[i]);
        taskarr[i].priority = i;
    }

    runningTask = &(taskarr[0]);
}

void initializer(void)
{

    taskSetup();
    heapInit();
    pendSVInit();
    uartInit();
    sysTickInit();
    main_to_task(taskarr[0].taskHandle);
}