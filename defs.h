
#define ICSR (*(unsigned long *)0xE000ED04)
#define PENDSVSET (1 << 28)
#define NULLT '\0'
#define NUMTASKS 3
#define PC_MASK 0xFFFFFFFE

void task1Handle(void);
void task2Handle(void);
void task3Handle(void);

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

void mutexTake(volatile struct Mutex *mutex);
void mutexReturn(volatile struct Mutex *mutex);
void displayLabel(const char *label);
void displayLine(void);
void displayHex(unsigned long num);
void taskInit(struct Task *task);
void enableInterrupts(void);
void disableInterrupts(void);
