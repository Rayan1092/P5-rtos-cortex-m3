
#define ICSR (*(unsigned long *)0xE000ED04)
#define PENDSVSET (1 << 28)
#define NULLT '\0'
#define NUMTASKS 4
#define PC_MASK 0xFFFFFFFE
#define GPIOCBASE ((volatile unsigned char *)0x40011000)
#define ODROFFSET 0xC
#define GPIOCODR (*(volatile unsigned long *)(GPIOCBASE + ODROFFSET))

void task1Handle(void);
void task2Handle(void);
void task3Handle(void);
void task4Handle(void);
void sendChar(char c);
void hardFaultHandler(void);
void displayStats();
void heapInit(void);
void readHeaders(void);
void *myMalloc(unsigned long size);
void myFree(void *ptr);
void yeild(void);
void uartTXHandle(void);
void uartInit(void);

enum taskState
{
    READYT,
    RUNNINGT,
    BLOCKEDT
};

struct Mutex
{
    unsigned long val;
    struct Task *holder;
};

struct Task
{
    unsigned long *sp;
    unsigned long stack[64];
    enum taskState state;
    void (*taskHandle)(void);
    volatile struct Mutex *awaitingMutex;
    // orginal priority
    unsigned long ogpriority;
    // effective priority
    unsigned long epriority;
};

extern volatile struct Mutex uartMutex, heapMutex;
extern struct Task *runningTask, taskarr[NUMTASKS];
extern int taskIndex;
extern unsigned long *GPIOCODR;

void mutexTake(volatile struct Mutex *mutex);
void mutexReturn(volatile struct Mutex *mutex);
void displayLabel(const char *label);
void displayLine(void);
void displayHex(unsigned long num);
void taskInit(struct Task *task);
void enableInterrupts(void);
void disableInterrupts(void);
