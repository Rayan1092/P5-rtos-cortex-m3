#include "defs.h"

#define UARTBASE ((volatile unsigned char *)0x40004000)
#define STATUS_OFFSET 0x4
#define CTRL_OFFSET 0x8
#define INTCLEAR_OFFSET 0xC
// interupt clear
#define INTCLEAR (*((volatile unsigned long *)(UARTBASE + INTCLEAR_OFFSET)))
#define CLEARTXINT (1 << 0)
#define CTRL (*((volatile unsigned long *)(UARTBASE + CTRL_OFFSET)))
#define DATA (*(volatile unsigned long *)UARTBASE)
// uart TX + Interupt enable
#define TXIEN (0x5 << 0)
#define STATUS_MASK (1 << 0)
#define IRQ 1
// NVIC allowing uart peripheral through (interupt set enable reg0)
#define ISER0 (*(volatile unsigned long *)0xE000E100)
#define UARTSTATUS (*((volatile unsigned long *)(UARTBASE + STATUS_OFFSET)))

volatile unsigned char ringbuffer[BUFFERSIZE];

volatile unsigned long writeIndex = 0;
volatile unsigned long readIndex = 0;

volatile unsigned long wLapVal = 0;
volatile unsigned long rLapVal = 0;

volatile unsigned long directSendFlag = 1;

unsigned long spaceAvaliable(void)
{
    return (wLapVal == rLapVal && writeIndex >= readIndex) || (wLapVal != rLapVal && writeIndex < readIndex);
}

void disableInterrupts(void)
{
    asm volatile(
        "mov r0, #1 \n"
        "msr primask, r0 \n" ::: "memory", "r0");
}

void enableInterrupts(void)
{
    asm volatile(
        "mov r0, #0 \n"
        "msr primask, r0 \n" ::: "memory", "r0");
}

void sendChar(char c)
{
    while (1)
    {
        disableInterrupts();

        if (directSendFlag)
        {
            DATA = c;
            directSendFlag = 0;

            enableInterrupts();
            break;
        }
        else if (spaceAvaliable())
        {

            ringbuffer[writeIndex] = c;
            writeIndex++;

            if (writeIndex >= BUFFERSIZE)
            {
                wLapVal = !wLapVal;
                writeIndex = 0;
            }
            enableInterrupts();

            break;
        }
        else
            enableInterrupts();

        while (!spaceAvaliable())
        {
        }
    }
}

void uartTXHandle(void)
{
    if (readIndex == writeIndex && rLapVal == wLapVal)
    {
        INTCLEAR = CLEARTXINT;
        directSendFlag = 1;
        return;
    }

    DATA = ringbuffer[readIndex];
    readIndex++;

    if (readIndex >= BUFFERSIZE)
    {
        readIndex = 0;
        rLapVal = !rLapVal;
    }

    INTCLEAR = CLEARTXINT;
}

void uartInit(void)
{
    ISER0 |= (1 << IRQ);
    CTRL |= TXIEN;
}
