void initializer(void);

int main(void)
{
    volatile unsigned long *garbage = (volatile unsigned long *)0xF0000000;
    *garbage = 1UL;
    initializer();
}