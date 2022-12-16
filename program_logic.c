#include "program_logic.h"

const useconds_t waitingTime = 50000;
const useconds_t nextIterationWaitingTime = 950000;
char **cpuNameStore = NULL;

void *loggerLoop()
{
    char *log;
    while (1)
    {
        log = receiveLog();
        while (NULL == log)
        {
            usleep(waitingTime);
            log = receiveLog();
        }
    }
}

void *readerLoop()
{
    enum SendingResult sr;
    while (1)
    {
        struct CpuReadData *readData = readProcStat(&cpuCount);
        sr = sendReadData(readData);
        while (FULL == sr)
        {
            usleep(waitingTime);
            sr = sendReadData(readData);
        }
        usleep(nextIterationWaitingTime);
    }
}

void *analyzerLoop()
{
    struct CpuReadData *readData;
    struct CpuUsage *usage;
    struct CpuTimeData *prevTimeData;
    enum SendingResult sr;
    readData = receiveReadData();
    while (NULL == readData)
    {
        usleep(waitingTime);
        readData = receiveReadData();
    }
    prevTimeData = startingTimeData(cpuCount);
    usage = analyze(readData, prevTimeData, cpuCount);
    free(readData);
    sr = sendUsage(usage);
    while (FULL == sr)
    {
        usleep(waitingTime);
        sr = sendUsage(usage);
    }
    usleep(nextIterationWaitingTime);
    while (1)
    {
        readData = receiveReadData();
        while (NULL == readData)
        {
            usleep(waitingTime);
            readData = receiveReadData();
        }
        usage = analyze(readData, prevTimeData, cpuCount);
        free(readData);
        sr = sendUsage(usage);
        while (FULL == sr)
        {
            usleep(waitingTime);
            sr = sendUsage(usage);
        }
        usleep(nextIterationWaitingTime);
    }
}

void *printerLoop()
{
    struct CpuUsage *usage;
    while (1)
    {
        usage = receiveUsage();
        while (NULL == usage)
        {
            usleep(waitingTime);
            usage = receiveUsage();
        }
        print(usage, cpuCount);
        free(usage);
        usleep(nextIterationWaitingTime);
    }
}

struct CpuReadData *readProcStat(int *cpuCount)
{
    char *rawData = readRawData();
    if (0 == *cpuCount)
    { // cpuCount is set to non-zero value only once
        *cpuCount = extraxtCpuCount(rawData);
    }
    if (NULL == cpuNameStore)
    {
        initCpuNameStore(*cpuCount);
    }
    struct CpuReadData *ret = extractDataFromRaw(rawData, *cpuCount);
    free(rawData);
    return ret;
}

struct CpuReadData *extractDataFromRaw(char *rawData, int cpuCount)
{
    struct CpuReadData *ret = (struct CpuReadData *)malloc(cpuCount * sizeof(struct CpuReadData));
    int offset = 0;
    for (int i = 0; i < cpuCount; ++i)
    {
        while ('\n' != *(rawData + offset++))
            ; // sets offfset to new line
        char *cpuName = getCpuName(i);
        char *temp = malloc(256 * sizeof(char));
        strcat(temp, cpuName);
        strcat(temp, " %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu");
        sscanf(&*(rawData + offset), temp,
               &(ret + i)->user, &(ret + i)->nice, &(ret + i)->system, &(ret + i)->idle,
               &(ret + i)->iowait, &(ret + i)->irq, &(ret + i)->softirq, &(ret + i)->steal,
               &(ret + i)->guest, &(ret + i)->guestnice);
        (ret + i)->index = i;
        free(temp);
    }

    return ret;
}

struct CpuUsage *analyze(struct CpuReadData *readData, struct CpuTimeData *previousTimeData, int cpuCount)
{
    struct CpuUsage *ret = (struct CpuUsage *)malloc(cpuCount * sizeof(struct CpuUsage));
    for (int i = 0; i < cpuCount; ++i)
    {
        unsigned long long Idle = (readData + i)->idle + (readData + i)->iowait;
        unsigned long long NonIdle = (readData + i)->user + (readData + i)->nice + (readData + i)->system +
                                     (readData + i)->irq + (readData + i)->softirq + (readData + i)->steal;
        unsigned long long Total = Idle + NonIdle;

        double dNonIdle = (double)NonIdle - (double)(previousTimeData + i)->NonIdle;
        double dTotal = (double)Total - (double)(previousTimeData + i)->Total;
        (ret + i)->usage = 100.0 * dNonIdle / dTotal;
        (ret + i)->index = i;

        // current time data will be previous time data in the future
        (previousTimeData + i)->Idle = Idle;
        (previousTimeData + i)->NonIdle = NonIdle;
        (previousTimeData + i)->Total = Total;
    }
    return ret;
}

void print(struct CpuUsage *data, int cpuCount)
{
    for(int i = 0; i < cpuCount; ++i)
    {
        char* cpuName = getCpuName((data+i)->index);
        printf("%s: %f%%, ", cpuName, (data + i)->usage);
    }
    printf("\n");
}

char *readRawData()
{
    FILE *file = fopen("/proc/stat", "r");
    if (NULL == file)
    {
        printf("file cannot be opened\n");
    }
    int max_len = 0x10000;
    char *rawData = malloc(max_len * sizeof(char));
    fread(rawData, sizeof(char), max_len, file);
    fclose(file);
    return rawData;
}

int extraxtCpuCount(char *rawData)
{
    int ret = 0;
    int offset = 0;
    while (1)
    {
        while ('\n' != *(rawData + offset++)) // first "cpu" occurence is omitted
            ;
        ++ret;
        if ('c' != *(rawData + offset) || 'p' != *(rawData + offset) || 'u' != *(rawData + offset))
        {
            break;
        }
    }
    return ret + 1;
}

struct CpuTimeData *startingTimeData(int cpuCount)
{
    struct CpuTimeData *ret = (struct CpuTimeData *)malloc(cpuCount * sizeof(struct CpuTimeData));
    for (int i = 0; i < cpuCount; ++i)
    {
        (ret + i)->index = 0;
        (ret + i)->Idle = 0;
        (ret + i)->NonIdle = 0;
        (ret + i)->Total = 0;
    }
    return ret;
}

void initCpuNameStore(int cpuCount)
{
    cpuNameStore = malloc(cpuCount * sizeof(char *));
    int numberOfDigits = getNumberOfDigits(cpuCount);
    int nameChars = 4 + numberOfDigits;
    for (int i = 0; i < cpuCount; ++i)
    {
        char *name = malloc(nameChars * sizeof(char));
        sprintf(name, "cpu%d", i);// matches naming in /proc/stat in the year 2022
        *(cpuNameStore + i) = name;
    }
}

char *getCpuName(int index)
{
    return *(cpuNameStore + index);
}

int getNumberOfDigits(int number) // designed for number>0
{
    int ret = 1;
    int divisor = 10;
    while (1)
    {
        if (number / divisor == 0)
        {
            return ret;
        }
        ++ret;
        divisor *= 10;
    }
}