#include "defs.h"

#define TIM3EN (1 << 1)
#define TIM3BASE ((volatile unsigned char *)0x40000400)
#define TIM3_CR1 (*((volatile unsigned long *)(TIM3BASE + CR1OFFSET)))
// encoder chanel A
#define PA6_FLOAT (0x4 << 24)
// encoder chanel B
#define PA7_FLOAT (0x4 << 28)
// slave mode reg offset
#define SMROFFSET 0x8
// slave mode control reg
#define TIM3_SMCR (*((volatile unsigned long *)(TIM3BASE + SMROFFSET)))
// slave mode selection 011 ()encoder mode 4x decoding remember the sequence
#define SMS (0x3 << 0)
// capture compare
#define TIM3_CCER (*((volatile unsigned long *)(TIM3BASE + CCEOFFSET)))
#define CCE1 (1 << 0)
#define CCE2 (1 << 4)
// both channels 1 and 2 polarity 00100010
#define CC12P (0x22 << 0)
#define TIM3_CCMR1 (*((volatile unsigned long *)(TIM3BASE + CCM1OFFSET)))
// select 1
#define CC1S (0x1 << 0)
#define CC2S (0x1 << 8)
// min 3.5 uS min pulse
#define IC1F (0xF << 4)
#define IC2F (0xF << 12)
// use all 16 available
#define ARRVALENC 0xFFFF
#define TIM3_ARR (*((volatile unsigned long *)(TIM3BASE + AROFFSET)))
// counter
#define CNTOFFSET 0x24
#define TIM3_CNTR (*((volatile unsigned long *)(TIM3BASE + CNTOFFSET)))

void encoderInit(void)
{
    RCC_APB1ENR |= TIM3EN;

    // cleared as reset val is 0x44444 (floating input) setting it later to be explicit
    GPIOA_CRL &= ~(0xF << 24 | 0xF << 28);
    GPIOA_CRL |= PA6_FLOAT | PA7_FLOAT;

    TIM3_SMCR |= SMS;

    TIM3_CCMR1 |= CC1S | CC2S | IC1F | IC2F;

    TIM3_CCER |= CCE1 | CCE2;
    TIM3_CCER &= ~(CC12P);

    TIM3_ARR = ARRVALENC;

    // enable counter once all is setup
    TIM3_CR1 |= CEN;
}

unsigned short readEncoder(void)
{
    return TIM3_CNTR;
}