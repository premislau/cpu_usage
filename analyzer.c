#include "analyzer.h"

const useconds_t dataWaitingTime = 50000;
const useconds_t nextIterationWaitingTime = 950000;

void *analyzerLoop()
{
    struct CpuReadData *readData;
    struct CpuUsage *usage;
    struct CpuTimeData *prevTimeData;
    enum SendingResult sr;
    readData = receiveReadData();
    while (NULL == readData)
    {
        usleep(dataWaitingTime);
        readData = receiveReadData();
    }
    prevTimeData = startingTimeData(cpuCount);
    usage = analyze(readData, prevTimeData, cpuCount);
    free(readData);
    sr = sendUsage(usage);
    while (FULL == sr)
    {
        usleep(dataWaitingTime);
        sr = sendUsage(usage);
    }
    usleep(nextIterationWaitingTime);
    while (1)
    {
        readData = receiveReadData();
        while (NULL == readData)
        {
            usleep(dataWaitingTime);
            readData = receiveReadData();
        }
        usage = analyze(readData, prevTimeData, cpuCount);
        free(readData);
        sr = sendUsage(usage);
        while (FULL == sr)
        {
            usleep(dataWaitingTime);
            sr = sendUsage(usage);
        }

        sr = sendActiveness(ANALYZER);
        //if (1 == watchdogActive)
        //{
            while (SUCCESS != sr)
            {
                usleep(dataWaitingTime);
                sr = sendActiveness(ANALYZER);
            }
        //}
        usleep(nextIterationWaitingTime);
    }
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