// reset and clock control, control reg
#define RCC_CR (*((volatile unsigned long *)0x40021000))
// high speed external on
#define HSEON (1 << 16)
#define HSERDY (1 << 17)
// flash access control reg
#define FLASH_ACR (*((volatile unsigned long *)0x40022000))
// set wait state (ceiling)
#define SETWSTATE (0x2 << 0)
#define CLEARWSTATE (~(0x7 << 0))
// prefetch buffer enable
#define PRFTBE (1 << 4)
#define RCC_CFGR (*(volatile unsigned long *)0x40021004)
// H prescaler (AHB andvanced high performance bus 72MHz after PLL)
#define SETHPRE (~(0xF << 4))
#define CLEARPPRE1 (~(0x7 << 8))
#define SETPPRE2 (~(0x7 << 11))
#define SETPPRE1 (0x4 << 8)
#define PLLSRC 16
#define PLLMUL 18
#define PLLON 24
#define PLLRDY (1 << 25)
// system clock switch
#define SW 0
#define PLL 0x2
// SW status
#define SWS (0x3 << 2)

void clockInit(void)
{
    RCC_CR |= HSEON;

    while (!(RCC_CR & HSERDY))
    {
    }

    FLASH_ACR &= CLEARWSTATE;
    FLASH_ACR |= SETWSTATE;

    FLASH_ACR |= PRFTBE;

    RCC_CFGR &= SETHPRE;
    RCC_CFGR &= CLEARPPRE1;
    RCC_CFGR |= SETPPRE1;
    RCC_CFGR &= SETPPRE2;

    // set to HSE
    RCC_CFGR |= (1 << PLLSRC);
    //  x 9 offset by 2
    RCC_CFGR |= (0x7 << PLLMUL);

    RCC_CR |= (1 << PLLON);

    while (!(RCC_CR & PLLRDY))
    {
    }

    // make sysclk use the pll
    RCC_CFGR |= (PLL << SW);

    while ((RCC_CFGR & SWS) != 0x8)
    {
    }
}
