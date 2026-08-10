#include "defs.h"
#define HEAPSIZE 240
#define USEFULSIZE 8
#define HEADERSIZE sizeof(struct Header)
extern unsigned long _ebss, _estack;

unsigned char *heapStart;
unsigned char *heapEnd;

enum heap_state
{
    ACTIVEMEM,
    FREEMEM
};

struct Header
{
    enum heap_state state;
    unsigned long payloadSize;
};

void heapInit(void)
{
    heapStart = (unsigned char *)&_ebss;

    while ((unsigned long)heapStart % 4 != 0)
    {
        heapStart++;
    }

    heapEnd = heapStart + HEAPSIZE;

    if (heapEnd > (unsigned char *)&_estack)
    {
        displayLabel("ERROR: \"HEAPEND\" OUT OF BOUNDS");
        while (1)
        {
        }
    }

    struct Header *header1 = (struct Header *)heapStart;
    header1->state = FREEMEM;
    header1->payloadSize = HEAPSIZE - HEADERSIZE;
}

void *myMalloc(unsigned long size)
{
    mutexTake(&heapMutex);

    struct Header *currHeapheader = (struct Header *)heapStart;

    while (size % 4 != 0)
    {
        size++;
    }

    unsigned long ogPayloadSize;
    // orginal heap header address
    unsigned long *OGHHA;

    while ((unsigned char *)currHeapheader < heapEnd)
    {
        if (currHeapheader->state == FREEMEM && currHeapheader->payloadSize >= size)
        {
            ogPayloadSize = currHeapheader->payloadSize;
            OGHHA = (unsigned long *)currHeapheader;
            currHeapheader->state = ACTIVEMEM;

            if (currHeapheader->payloadSize >= HEADERSIZE + size + USEFULSIZE)
            {
                currHeapheader->payloadSize = size;
                currHeapheader = (struct Header *)(((unsigned char *)currHeapheader) + size + HEADERSIZE);
                currHeapheader->state = FREEMEM;
                currHeapheader->payloadSize = ogPayloadSize - size - HEADERSIZE;
            }
            else
            {
                currHeapheader->payloadSize = ogPayloadSize;
            }

            mutexReturn(&heapMutex);
            return (void *)(((unsigned char *)OGHHA) + HEADERSIZE);
        }

        else
        {
            currHeapheader = (struct Header *)(((unsigned char *)currHeapheader) + currHeapheader->payloadSize + HEADERSIZE);
        }
    }
    mutexReturn(&heapMutex);
    return 0;
}

void readHeaders(void)
{
    mutexTake(&uartMutex);
    mutexTake(&heapMutex);

    struct Header *currHeader = (struct Header *)heapStart;
    struct Header *prevHeader;

    do
    {
        unsigned long payloadSize = currHeader->payloadSize;

        displayLabel("HEADER: ");
        if (currHeader->state == FREEMEM)
            displayLabel("STATE = FREEMEM  ");
        else
            displayLabel("STATE = ACTIVEMEM  ");

        displayLabel("PAYLOADSIZE = ");
        displayHex(payloadSize);

        displayLine();

        prevHeader = currHeader;

        currHeader = (struct Header *)(((unsigned char *)currHeader) + payloadSize + HEADERSIZE);
    } while ((unsigned char *)currHeader < heapEnd);

    displayLine();
    displayLabel("HEAP HEADERS DISPLAYED");
    displayLine();

    mutexReturn(&uartMutex);
    mutexReturn(&heapMutex);
}

void myFree(void *ptr)
{
    mutexTake(&heapMutex);

    // todo base header
    struct Header *baseH;
    int totalSize;

    struct Header *currHeader = (struct Header *)heapStart;

    struct Header *headerGiven = (struct Header *)ptr;
    headerGiven--;

    headerGiven->state = FREEMEM;

    unsigned long currpayloadSize;

    while ((unsigned char *)currHeader < heapEnd)
    {
        if (currHeader->state == FREEMEM)
        {
            currpayloadSize = currHeader->payloadSize;
            baseH = currHeader;
            totalSize = currHeader->payloadSize;

            currHeader = (struct Header *)(((unsigned char *)currHeader) + currpayloadSize + HEADERSIZE);

            while ((unsigned char *)currHeader < heapEnd && currHeader->state == FREEMEM)
            {
                currpayloadSize = currHeader->payloadSize;

                totalSize += currpayloadSize + HEADERSIZE;
                currHeader = (struct Header *)(((unsigned char *)currHeader) + currpayloadSize + HEADERSIZE);
            }
            if (totalSize == baseH->payloadSize)
                continue;

            baseH->payloadSize = totalSize;
        }
        else
        {
            currpayloadSize = currHeader->payloadSize;
            currHeader = (struct Header *)(((unsigned char *)currHeader) + currpayloadSize + HEADERSIZE);
        }
    }
    mutexReturn(&heapMutex);
}
