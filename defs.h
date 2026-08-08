#define UARTBASE ((volatile unsigned char *)0x40004000)

struct Task
{
    unsigned long *sp;
    unsigned long stack[256];
};

extern struct Task task1, task2, *runningTask;
void task1Handle(void);
void task2Handle(void);

void mutexTake(volatile unsigned long *mutex);
void mutexReturn(volatile unsigned long *mutex);

extern volatile unsigned long uartMutex;

void sendChar(char c);

void hardFaultHandler(void);