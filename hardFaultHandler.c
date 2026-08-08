#include "defs.h"

#define HEXMASK 0xF
#define CFSR (*(unsigned long *)0xE000ED28)
#define HFSR (*(unsigned long *)0xE000ED2C)
#define MMFAR (*(unsigned long *)0xE000ED34)
#define BFAR (*(unsigned long *)0xE000ED38)
#define MMFARVALID (1 << 7)
#define BFARVALID (1 << 15)

void displayLine(void)
{
    sendChar('\n');
}

void displayLabel(const char *label)
{
    unsigned char mval = *label;

    while (mval != '\0')
    {
        sendChar(mval);
        label++;
        mval = *label;
    }
}

void displayHex(unsigned long num)
{
    for (int i = 28; i >= 0; i -= 4)
    {
        unsigned long val = (num >> i) & HEXMASK;

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

// r0 already loaded with sp
void displayStats(unsigned long *frame)
{
    unsigned long PC = frame[6];
    displayLabel("PC=0x");
    displayHex(PC);
    displayLine();

    displayLabel("CFSR=");
    displayBinary(CFSR);
    displayLine();

    displayLabel("HFSR=");
    displayBinary(HFSR);
    displayLine();

    if (CFSR & MMFARVALID)
    {
        displayLabel("MMAR=");
        displayHex(MMFAR);
        displayLine();
    }

    if (CFSR & BFARVALID)
    {
        displayLabel("BFAR=");
        displayHex(BFAR);
        displayLine();
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
        "b CFunction \n"
        "skipPSP: \n"
        "mrs r0, msp \n"
        "CFunction: \n"
        "b displayStats \n"

    );
}