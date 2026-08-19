#include "defs.h"

#define USARTBASE ((volatile unsigned char *)0x40013800)
#define CR1OFFSET 0xC
#define USART_CR1 (*((volatile unsigned long *)(USARTBASE + CR1OFFSET)))
#define USARTEN (1 << 13)
// TX Enable
#define TE (1 << 3)
// TX empty interupt enable
#define TXEIE (1 << 7)
#define BRROFFSET 0x8
#define USART_BRR (*((volatile unsigned long *)(USARTBASE + BRROFFSET)))
#define GPIOA_BASE ((volatile unsigned char *)0x40010800)
#define GPIOA_CRH (*((volatile unsigned long *)(GPIOA_BASE + CRHOFFSET)))
// part of the NVIC
#define ISER1 (*((volatile unsigned long *)0xE000E104))
#define DROFFSET 0x4
#define USART_DR (*((volatile unsigned long *)(USARTBASE + DROFFSET)))

unsigned long prime = 1;

unsigned long writeIndex = 0;
unsigned long readIndex = 0;

unsigned long wlap = 0;
unsigned long rlap = 0;

unsigned char ringB[BUFFERSIZE];

void usartInit(void)
{
    // holds divisor 1/16 (72MHZ / 115200 = 625) ((39 + 1/16) x 16) = 625 (samples it 16x)
    USART_BRR = (39 << 4) | 1;

    USART_CR1 |= USARTEN | TE;

    // CNF MSB + mid 10 alternate function push pull, MODE 11 output 50MHZ PA9
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |= (0xB << 4);

    // allow USART1 interupts to go through IRQ 37
    ISER1 |= (1 << 5);
}

unsigned long spaceAvailable(void)
{
    return (writeIndex >= readIndex && wlap == rlap) || (writeIndex < readIndex && wlap != rlap);
}

void sendChar(char c)
{
    while (1)
    {
        disableInterrupts();

        if (prime)
            USART_CR1 |= TXEIE;

        if (spaceAvailable())
        {
            ringB[writeIndex] = c;
            writeIndex++;

            if (writeIndex >= BUFFERSIZE)
            {
                writeIndex = 0;
                wlap = !wlap;
            }
            enableInterrupts();
            break;
        }
        else
        {
            enableInterrupts();

            while (!spaceAvailable())
            {
            }
        }
    }
}

void usartTXEHandle(void)
{

    if (readIndex == writeIndex && wlap == rlap)
    {
        prime = 1;
        USART_CR1 &= ~TXEIE;
        return;
    }
    else
        prime = 0;

    USART_DR = ringB[readIndex];
    readIndex++;

    if (readIndex >= BUFFERSIZE)
    {
        readIndex = 0;
        rlap = !rlap;
    }
}