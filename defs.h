#define UARTBASE ((volatile unsigned char *)0x40004000)

struct Task
{
    unsigned long *sp;
    unsigned long stack[64];
};

extern struct Task task1, task2, *runningTask;

void task1Handle(void);
void task2Handle(void);