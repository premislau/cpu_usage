#include "program_logic.c"

int main();
int multiThreaded();
int singleThreaded();

int main()
{
    return multiThreaded();
    //return singleThreaded();
}

int multiThreaded()
{
    initReadDataMailbox(2);
    initUsageMailbox(2);
    pthread_t readerId, analyzerId, printerId;
    int err;
    err = pthread_create(&readerId, NULL, &readerLoop, NULL);
    if (err)
    {
        printf("Erorr: %d\n", err);
        return 1;
    }
    err = pthread_create(&analyzerId, NULL, &analyzerLoop, NULL);
    if (err)
    {
        printf("Erorr: %d\n", err);
        return 1;
    }
    err = pthread_create(&printerId, NULL, &printerLoop, NULL);
    if (err)
    {
        printf("Erorr: %d\n", err);
        return 1;
    }
    pthread_join(readerId, NULL);
    pthread_join(analyzerId, NULL);
    pthread_join(printerId, NULL);
    return 0;
}

int singleThreaded()
{
    int cpuCount = 0;
    useconds_t sleepTime = 50000;
    struct CpuReadData *readData = readProcStat(&cpuCount);
    struct CpuTimeData *prevTimeData = startingTimeData(cpuCount);
    struct CpuUsage *usage = analyze(readData, prevTimeData, cpuCount);
    free(readData);
    print(usage, cpuCount);
    free(usage);
    usleep(sleepTime);
    while(1)
    {
        readData = readProcStat(&cpuCount);
        usage = analyze(readData, prevTimeData, cpuCount);
        free(readData);
        print(usage, cpuCount);
        free(usage);
        usleep(sleepTime);
    }
    free(prevTimeData);
    return 0;
}
