extern unsigned long _estack;

void start_up(void);
void pendSVHandle(void);

__attribute__((section(".isr_vector"))) void (*vector_table[16])(void) = {
    (void (*)(void))&_estack,
    start_up,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    pendSVHandle,
    0};