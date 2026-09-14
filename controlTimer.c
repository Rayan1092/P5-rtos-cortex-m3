#include "defs.h"
#define TIM4BASE ((volatile unsigned char *)0x40000800)
#define CR1OFFSET 0x00
// DMA/Interupt Enable Register
#define DIEROFFSET 0x0C
// prescaler
#define PSCOFFSET 0x28
#define ARROFFSET 0x2C
#define SROFFSET 0x10

#define TIM4CR1 (*((volatile unsigned long *)(TIM4BASE + CR1OFFSET)))
#define TIM4DIER (*((volatile unsigned long *)(TIM4BASE + DIEROFFSET)))
#define TIM4PSC (*((volatile unsigned long *)(TIM4BASE + PSCOFFSET)))
#define TIM4ARR (*((volatile unsigned long *)(TIM4BASE + ARROFFSET)))
#define TIM4SR (*((volatile unsigned long *)(TIM4BASE + SROFFSET)))
// Counter EN
#define CEN (1 << 0)
// Update
#define UIE (1 << 0)
#define TIM4EN (1 << 2)
// NVIC EN
#define NVICEN (1 << IRQT4)
// starts at 0 hence 72. 72,000,000 / 72 = 1,000,000 (main counter "+" every 1,0000,000 cycles, 1us)
#define PSC 71
// x * 0.000001  = 0.01 so x = 10000 (0-9999)
#define ARR 9999
// Update Interupt Flag
#define UIF (1 << 0)

void tim4Init(void)
{
    RCC_APB1ENR |= TIM4EN;

    TIM4PSC = PSC;
    TIM4ARR = ARR;

    TIM4DIER |= UIE;
    TIM4CR1 |= CEN;

    // last after all is done
    ISER0 |= NVICEN;
}

void tim4ISR(void)
{
    struct Task *task1 = &taskarr[0];

    if (task1->state == BLOCKEDT)
        task1->state = READYT;

    TIM4SR &= ~UIF;
    ICSR |= PENDSVSET;
}