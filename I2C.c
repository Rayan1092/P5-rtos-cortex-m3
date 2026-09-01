#include "defs.h"

#define I2C1BASE ((volatile unsigned char *)0x40005400)
#define I2CEN (1 << 21)
#define IOPBEN (1 << 3)
#define GPIOBBASE ((volatile unsigned char *)0x40010C00)
#define GPIOB_CRL (*((volatile unsigned long *)(GPIOBBASE + CRLOFFSET)))
// SCL (50MHZ out open drain)
#define PB6 (0xF << 24)
// SDA
#define PB7 (0xF << 28)
// 36MHZ remember on APB1 hence 36
#define FREQ (36 << 0)
#define CR2OFFSET 0x04
#define I2C1_CR2 (*((volatile unsigned long *)(I2C1BASE + CR2OFFSET)))
// clock control not capture compare
#define CCROFFSET 0x1C
#define I2C1_CCR (*((volatile unsigned long *)(I2C1BASE + CCROFFSET)))
// set fast mode 400 kHZ (standard 100kHz)
#define SETFAST (1 << 15)
// 0 (low twice as long as high 2:1)
#define DUTY (~(1 << 14))
// 0-11 holds one segment (3 total since duty is 2:1) (36000000 / 400000) / 3 for SCL BTW
#define DIVISOR 30
// max time to rise from low to high
#define TRISEOFFSET 0x20
#define I2C1_TRISE (*((volatile unsigned long *)(I2C1BASE + TRISEOFFSET)))
// 300ns (max time allowed) / 27.8 (time for 1 cycle (36MHZ) = 10 then add 1 as per manual)
#define TRISE 11
#define CR1OFFSET 0x0
#define I2C1_CR1 (*((volatile unsigned long *)(I2C1BASE + CR1OFFSET)))
// peripheral enable
#define PE (1 << 0)
#define PD (~(1 << 0))

#define INAADDR 0x40
#define START (1 << 8)
#define DROFFSET 0x10
#define I2C1_DR (*((volatile unsigned long *)(I2C1BASE + DROFFSET)))
#define TXE (1 << 7)
#define SR1OFFSET 0x14
#define I2C1_SR1 (*((volatile unsigned long *)(I2C1BASE + SR1OFFSET)))
#define ADDR (1 << 1)
// ack failure
#define AF (1 << 10)
#define SR2OFFSET 0x18
#define I2C1_SR2 (*((volatile unsigned long *)(I2C1BASE + SR2OFFSET)))
// INA228 Current reg
#define INACR 0x07
// byte transfer finished + ack
#define BTF (1 << 2)
#define RXNE (1 << 6)
#define ACK (1 << 10)
#define STOP (1 << 9)
// timeout iterations
#define TIMITER 5000
// start bit (until set)
#define SB (1 << 0)
#define RSHUNT 0.01
// the resolution were measuring at (3A MAX, 19bits (524,287 max val)) hence 3/524287
#define CURRENT_LSB 0.000005722
// TI formula 13107.2 x 10^6 x CURRENT_LSB x Rshunt
#define ShuntVal 750

void i2cInit(void)
{

    RCC_APB1ENR |= I2CEN;
    RCC_APB2ENR |= IOPBEN;

    // already off but to be certain
    I2C1_CR1 &= PD;

    GPIOB_CRL &= ~((0xF << 24) | (0xF << 28));
    GPIOB_CRL |= PB6 | PB7;

    I2C1_CR2 |= FREQ;

    I2C1_CCR |= SETFAST | DIVISOR;
    I2C1_CCR &= DUTY;

    I2C1_TRISE = TRISE;

    I2C1_CR1 |= PE;
}

unsigned long sendStart(void)
{
    I2C1_CR1 |= START;
    unsigned long timeoutCount = 0;

    while (!(I2C1_SR1 & SB))
    {
        if (timeoutCount >= TIMITER)
        {
            return 0;
        }
        timeoutCount++;
    }
    return 1;
}

// To use this function count must be >= 3 as a 2 byte read requires a differnt sequence as per ST
unsigned long readReg(unsigned char addr, unsigned char regNum, unsigned char count, unsigned long *status)
{
    unsigned char byte = (addr << 1) | 0;
    unsigned long timeoutCount = 0;

    if (!sendStart())
    {
        I2C1_CR1 |= STOP;
        *status = 0;
        return 0;
    }

    I2C1_DR = byte;

    while (1)
    {
        if (timeoutCount >= TIMITER)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            I2C1_CR1 |= STOP;
            *status = 0;
            return 0;
        }

        if (I2C1_SR1 & ADDR)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            break;
        }
        else if (I2C1_SR1 & AF)
        {
            I2C1_SR1 &= ~AF;
            // clear ADDR (must read SR1 & 2)
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            I2C1_CR1 |= STOP;
            *status = 0;
            return 0;
        }
        timeoutCount++;
    }
    timeoutCount = 0;

    while (!(I2C1_SR1 & TXE))
    {
        if (timeoutCount >= TIMITER)
        {
            I2C1_CR1 |= STOP;
            *status = 0;
            return 0;
        }
        timeoutCount++;
    }
    timeoutCount = 0;

    I2C1_DR = regNum;

    while (1)
    {
        if (timeoutCount >= TIMITER)
        {
            I2C1_CR1 |= STOP;
            *status = 0;
            return 0;
        }
        if (I2C1_SR1 & BTF)
            break;

        else if (I2C1_SR1 & AF)
        {
            I2C1_SR1 &= ~AF;
            I2C1_CR1 |= STOP;
            *status = 0;
            return 0;
        }
    }
    timeoutCount = 0;

    // ack all incoming bytes
    I2C1_CR1 |= ACK;

    if (!sendStart())
    {
        I2C1_CR1 |= STOP;
        *status = 0;
        return 0;
    }

    byte = (addr << 1) | 1;
    I2C1_DR = byte;

    while (1)
    {
        if (timeoutCount >= TIMITER)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            I2C1_CR1 |= STOP;
            *status = 0;
            return 0;
        }

        if (I2C1_SR1 & ADDR)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            break;
        }
        else if (I2C1_SR1 & AF)
        {
            I2C1_SR1 &= ~AF;
            // clear ADDR (must read SR1 & 2)
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            I2C1_CR1 |= STOP;
            *status = 0;
            return 0;
        }
        timeoutCount++;
    }
    timeoutCount = 0;

    unsigned long data = 0;
    unsigned char tempData;
    unsigned char bytesRead = 0;
    unsigned char shiftVal = 24;

    while (bytesRead < count)
    {
        if (I2C1_SR1 & RXNE)
        {
            tempData = (unsigned char)I2C1_DR;
            data |= ((unsigned long)tempData << shiftVal);
            shiftVal -= 8;
            bytesRead++;

            if (bytesRead == count - 1)
            {
                I2C1_CR1 &= ~ACK;
                // schedule stop not insta
                I2C1_CR1 |= STOP;
            }
        }

        if (timeoutCount >= (count * TIMITER))
        {
            I2C1_CR1 |= STOP;
            *status = 0;
            return 0;
        }
        timeoutCount++;
    }
    I2C1_CR1 |= STOP;

    // + 8 as we decremented it last iter
    *status = 1;
    return (data >> (shiftVal + 8));
}

void writeReg(unsigned char addr, unsigned char regNum, unsigned long data, unsigned char count, unsigned long *status)
{
    unsigned char byte = (addr << 1) | 0;
    unsigned long timeoutCount = 0;

    if (!sendStart())
    {
        I2C1_CR1 |= STOP;
        *status = 0;
        return;
    }

    I2C1_DR = byte;

    while (1)
    {
        if (timeoutCount >= TIMITER)
        {
            *status = 0;
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            I2C1_CR1 |= STOP;
            return;
        }

        if (I2C1_SR1 & ADDR)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            break;
        }

        else if (I2C1_SR1 & AF)
        {
            *status = 0;
            I2C1_CR1 |= STOP;
            return;
        }
        timeoutCount++;
    }

    timeoutCount = 0;

    while (!(I2C1_SR1 & TXE))
    {
        if (timeoutCount >= TIMITER)
        {
            *status = 0;
            I2C1_CR1 |= STOP;
            return;
        }
        timeoutCount++;
    }

    I2C1_DR = regNum;
    timeoutCount = 0;

    while (1)
    {
        if (I2C1_SR1 & BTF)
        {
            break;
        }

        if (I2C1_SR1 & AF)
        {
            I2C1_SR1 &= ~AF;
            *status = 0;
            I2C1_CR1 |= STOP;
            return;
        }

        if (timeoutCount >= TIMITER)
        {
            *status = 0;
            I2C1_CR1 |= STOP;
            return;
        }
        timeoutCount++;
    }
    timeoutCount = 0;

    unsigned char sentData = 0;
    // timout val
    unsigned long tVal = TIMITER * count;

    while (count > 0)
    {
        if (I2C1_SR1 & TXE)
        {

            sentData = (unsigned char)(data >> ((8 * count) - 8));
            I2C1_DR = sentData;
            count--;
        }

        if (timeoutCount >= tVal)
        {
            *status = 0;
            I2C1_CR1 |= STOP;
            return;
        }
        timeoutCount++;
    }
    timeoutCount = 0;

    while (!(I2C1_SR1 & BTF))
    {
        if (timeoutCount >= TIMITER)
        {
            *status = 0;
            I2C1_CR1 |= STOP;
            return;
        }

        timeoutCount++;
    }
    I2C1_CR1 |= STOP;

    *status = 1;
    return;
}