#include "program_logic.h"
const useconds_t waitingTime = 1000;
const useconds_t nextIterationWaitingTime = 100000;

void *loggerLoop(void *arg)
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

void *readerLoop(void *arg)
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

void *analyzerLoop(void *arg)
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
    sr = sendUsage(usage);
    while (FULL == sr)
    {
        usleep(waitingTime);
        sr = sendUsage(usage);
    }
    free(readData);
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
        sr = sendUsage(usage);
        while (FULL == sr)
        {
            usleep(waitingTime);
            sr = sendUsage(usage);
        }
        free(readData);
        usleep(nextIterationWaitingTime);
    }
}

void *printerLoop(void *arg)
{
    int i = 0;
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
    struct CpuReadData *ret = extractDataFromRaw(rawData, *cpuCount);
    free(rawData);
    return ret;
}

struct CpuReadData *extractDataFromRaw(char *rawData, int cpuCount)
{
    struct CpuReadData *ret = (struct CpuReadData *)malloc(cpuCount * sizeof(struct CpuReadData));
    int offset = 0;
    int cpuIndex = 0;
    while (cpuIndex < cpuCount)
    {
        while ('\n' != *(rawData + offset++))
            ; // sets offfset to new line
        char *cpu_id = malloc(9 * sizeof(char));
        sprintf(cpu_id, "cpu%d", cpuIndex);
        char *temp = malloc(256 * sizeof(char));
        strcat(temp, cpu_id);
        strcat(temp, " %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu");
        int sscanfResult = sscanf(&*(rawData + offset), temp,
                                  &(ret + cpuIndex)->user, &(ret + cpuIndex)->nice, &(ret + cpuIndex)->system, &(ret + cpuIndex)->idle,
                                  &(ret + cpuIndex)->iowait, &(ret + cpuIndex)->irq, &(ret + cpuIndex)->softirq, &(ret + cpuIndex)->steal,
                                  &(ret + cpuIndex)->guest, &(ret + cpuIndex)->guestnice);
        (ret + cpuIndex)->cpu_id = cpu_id;
        free(temp);
        ++cpuIndex;
    }

    return ret;
}

struct CpuUsage *analyze(struct CpuReadData *readData, struct CpuTimeData *previousTimeData, int cpuCount)
{
    struct CpuUsage *ret = (struct CpuUsage *)malloc(cpuCount * sizeof(struct CpuUsage));
    struct CpuTimeData *newTimeData;
    int offset = 0;
    int cpu_index = 0;
    while (cpu_index < cpuCount)
    {
        unsigned long long Idle = (readData + cpu_index)->idle + (readData + cpu_index)->iowait;
        unsigned long long NonIdle = (readData + cpu_index)->user + (readData + cpu_index)->nice + (readData + cpu_index)->system +
                                     (readData + cpu_index)->irq + (readData + cpu_index)->softirq + (readData + cpu_index)->steal;
        unsigned long long Total = Idle + NonIdle;

        double dNonIdle = (double)NonIdle - (double)(previousTimeData + cpu_index)->NonIdle;
        double dTotal = (double)Total - (double)(previousTimeData + cpu_index)->Total;
        (ret + cpu_index)->usage = 100.0 * dNonIdle / dTotal;
        (ret + cpu_index)->cpu_id = (readData + cpu_index)->cpu_id;

        // current time data will be previous time data in the future
        (previousTimeData + cpu_index)->Idle = Idle;
        (previousTimeData + cpu_index)->NonIdle = NonIdle;
        (previousTimeData + cpu_index)->Total = Total;
        ++cpu_index;
    }

    return ret;
}

void print(struct CpuUsage *data, int cpuCount)
{
    int cpu_index = 0;
    while (cpu_index < cpuCount)
    {
        printf("%s: %f, ", (data + cpu_index)->cpu_id, (data + cpu_index)->usage);
        ++cpu_index;
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
        char *cpu_id = malloc(9 * sizeof(char));
        sprintf(cpu_id, "cpu%d", i);
        (ret + i)->cpu_id = cpu_id;
        (ret + i)->Idle = 0;
        (ret + i)->NonIdle = 0;
        (ret + i)->Total = 0;
    }
    return ret;
}