extern unsigned long _estack;

void hardFaultHandler(void);
void start_up(void);
void pendSVHandle(void);
void sysTickHandler(void);
void usartTXEHandle(void);

__attribute__((section(".isr_vector"))) void (*vectorTable[54])(void) = {
    [0] = (void (*)(void))&_estack,
    [1] = start_up,
    [3] = hardFaultHandler,
    [14] = pendSVHandle,
    [15] = sysTickHandler,
    [53] = usartTXEHandle};