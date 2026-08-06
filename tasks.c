#include "defs.h"

#define STATUS_OFFSET 0x4
#define STATUS_MASK (1 << 0)
#define ICSR (*(unsigned long *)0xE000ED04)
#define PENDSVSET (1 << 28)

struct Task task1;
struct Task task2;

struct Task *runningTask;

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
    while (1)
    {
        mutexTake(&uartMutex);
        sendChar('G');
        sendChar('G');
        sendChar('G');
        mutexReturn(&uartMutex);
    }
}

void task2Handle(void)
{
    while (1)
    {
        mutexTake(&uartMutex);
        sendChar('R');
        sendChar('R');
        sendChar('R');
        mutexReturn(&uartMutex);
    }
}

void nextTask(void)
{
    if (runningTask == &task1)
        runningTask = &task2;
    else
        runningTask = &task1;
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
