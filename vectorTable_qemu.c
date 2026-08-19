
extern unsigned long _estack;

void hardFaultHandler(void);
void start_up(void);
void pendSVHandle(void);
void sysTickHandler(void);
void uartTXHandle(void);

__attribute__((section(".isr_vector"))) void (*vector_table[18])(void) = {
    [0] = (void (*)(void))&_estack,
    [1] = start_up,
    [3] = hardFaultHandler,
    [14] = pendSVHandle,
    [15] = sysTickHandler,
    [17] = uartTXHandle};