#include "defs.h"

#define CFSR (*(unsigned long *)0xE000ED28)
#define HFSR (*(unsigned long *)0xE000ED2C)
#define BFAR (*(unsigned long *)0xE000ED38)
#define MMFAR (*(unsigned long *)0xE000ED34)
#define MMFARVALID (1 << 7)
#define BFARVALID (1 << 15)

void displayHex(unsigned long num)
{
    unsigned long mask = 0xF;

    for (int i = 28; i >= 0; i -= 4)
    {
        unsigned long val = (num >> i) & mask;

        if (val <= 9)
            sendChar('0' + val);

        else
            sendChar('A' + (val - 10));
    }
}

void displayBinary(unsigned long num)
{

    for (int i = 31; i >= 0; i--)
    {
        if (num & (1UL << i))
            sendChar('1');
        else
            sendChar('0');
    }
}

void displayLabel(const char *mssg)
{
    unsigned char val = *mssg;

    while (val != '\0')
    {
        sendChar(val);
        mssg++;
        val = *mssg;
    }
}

void displayNewLine(void)
{
    sendChar('\n');
}

void displayFaultStats(unsigned long *frame)
{
    unsigned long PC = frame[6];

    displayLabel("PC=0x");
    displayHex(PC);

    displayNewLine();

    displayLabel("CFSR=");
    displayBinary(CFSR);

    displayNewLine();

    displayLabel("HFSR=");
    displayBinary(HFSR);

    displayNewLine();

    if (CFSR & MMFARVALID)
    {
        displayLabel("MMFAR=");
        displayHex(MMFAR);
        displayNewLine();
    }

    if (CFSR & BFARVALID)
    {
        displayLabel("BFAR=");
        displayHex(BFAR);
    }

    while (1)
    {
    }
}

__attribute__((naked)) void hardFaultHandler(void)
{
    asm volatile(
        "ands r0, lr, #4 \n"
        "beq skipPSP \n"
        "mrs r0, psp \n"
        "b loadCFunction \n"
        "skipPSP: \n"
        "mrs r0, msp \n"
        "loadCFunction: \n"
        "b displayFaultStats \n");
}
