extern unsigned long _sdata, _edata, _sidata, _sbss, _ebss;

int main(void);

void start_up(void)
{
    unsigned long *src = &_sidata;
    unsigned long *dst = &_sdata;

    while (dst != &_edata)
    {
        *dst = *src;
        dst++;
        src++;
    }

    dst = &_sbss;

    while (dst != &_ebss)
    {
        *dst = 0;
        dst++;
    }

    main();

    // make sure we dont go into UB
    while (1)
    {
    }
}