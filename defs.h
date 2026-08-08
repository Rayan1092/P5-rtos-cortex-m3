#define UARTBASE ((volatile unsigned char *)0x40004000)
#define NULLT '\0'
#define NUMTASKS 3

void task1Handle(void);
void task2Handle(void);
void task3Handle(void);

void mutexTake(volatile unsigned long *mutex);
void mutexReturn(volatile unsigned long *mutex);
void sendChar(char c);
void hardFaultHandler(void);

enum taskState
{
    READYT,
    RUNNINGT,
    BLOCKEDT
};

struct Task
{
    unsigned long *sp;
    unsigned long stack[64];
    enum taskState state;
    void (*taskHandle)(void);
};

extern volatile unsigned long uartMutex;
extern struct Task *runningTask, taskarr[NUMTASKS];
extern int taskIndex;
