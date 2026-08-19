#include "defs.h"

#define TIM2EN (1 << 0)
#define TIM2BASE ((volatile unsigned char *)0x40000000)
#define TIM2_CCMR1 (*((volatile unsigned long *)(TIM2BASE + CCM1OFFSET)))
// output
#define CC1S (~(0x3 << 0))
#define CC2S (~(0x3 << 8))
// prelad enable (wait till cycle is done)
#define OC1PE (1 << 3)
#define OC2PE (1 << 11)
// pwm mode 4-6
#define OC1M (0x6 << 4)
#define OC2M (0x6 << 12)
#define TIM2_CCER (*((volatile unsigned long *)(TIM2BASE + CCEOFFSET)))
#define CC1E (1 << 0)
#define CC2E (1 << 4)
#define CC12P (~(0x22 << 0))
// alternate function push pull
#define PA0 (0xB << 0)
#define PA1 (0xB << 4)
#define TIM2_ARR (*((volatile unsigned long *)(TIM2BASE + AROFFSET)))
#define TIM2_CR1 (*((volatile unsigned long *)(TIM2BASE + CR1OFFSET)))
#define CCR1OFFSET 0x34
#define CCR2OFFSET 0x38
#define TIM2_CCR1 (*((volatile unsigned long *)(TIM2BASE + CCR1OFFSET)))
#define TIM2_CCR2 (*((volatile unsigned long *)(TIM2BASE + CCR2OFFSET)))

void pwmInit(void)
{
    RCC_APB1ENR |= TIM2EN;

    TIM2_CCER |= CC1E | CC2E;
    TIM2_CCER &= CC12P;

    // why clear explained in encoder file
    GPIOA_CRL &= ~((0xF << 0) | (0xF << 4));
    GPIOA_CRL |= PA0 | PA1;

    TIM2_ARR = ARRVALPWM;

    TIM2_CCMR1 &= CC1S | CC2S;
    TIM2_CCMR1 |= OC1M | OC2M | OC1PE | OC2PE;

    TIM2_CR1 |= CEN;
}

void setDutyC(unsigned long val, unsigned long direction)
{
    if (direction)
    {
        TIM2_CCR1 = val;
        TIM2_CCR2 = 0;
    }
    else
    {
        TIM2_CCR1 = 0;
        TIM2_CCR2 = val;
    }
}