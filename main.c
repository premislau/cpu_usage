#include "program_logic.c"

int main();
int multiThreaded();
int singleThreaded();
struct WatchdogArgs *getWatchdogArgs(int, pthread_t, pthread_t, pthread_t);

int main()
{
    return multiThreaded();
    // return singleThreaded();
}


int multiThreaded()
{
    watchdogActive = 1;
    char* logFile = "logs.txt";
    initReadDataMailbox(2);
    initUsageMailbox(2);
    initActivenessMailbox(64); //should be big enough to fit all activity messages
    initLogMailbox(16);

    pthread_t readerId, analyzerId, printerId;
    pthread_t loggerId;
    int err;
    err = pthread_create(&loggerId, NULL, &loggerLoop, (void*) logFile);
    if (err)
    {
        printf("Erorr: %d\n", err);
        return 1;
    }
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

    struct WatchdogArgs *watchdogArgs = getWatchdogArgs(3, readerId, analyzerId, printerId);
    pthread_t watchdogId;
    err = pthread_create(&watchdogId, NULL, &watchdogLoop, (void*) watchdogArgs);
    if (err)
    {
        printf("Erorr: %d\n", err);
        return 1;
    }

    pthread_join(loggerId, NULL);
    pthread_join(readerId, NULL);
    pthread_join(analyzerId, NULL);
    pthread_join(printerId, NULL);


    pthread_join(watchdogId, NULL);

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
    while (1)
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

struct WatchdogArgs *getWatchdogArgs(int numberOfThreads, pthread_t readerId, pthread_t analyzerId, pthread_t printerId)
{
    struct WatchdogArgs *watchdogArgs = malloc(sizeof(struct WatchdogArgs));
    watchdogArgs->numberOfThreads = numberOfThreads;
    watchdogArgs->threads = malloc(numberOfThreads * sizeof(struct WatchedThread));
    watchdogArgs->threads->threadId = readerId;
    watchdogArgs->threads->threadType = READER;
    ((watchdogArgs->threads) + 1)->threadId = analyzerId;
    ((watchdogArgs->threads) + 1)->threadType = ANALYZER;
    ((watchdogArgs->threads) + 2)->threadId = printerId;
    ((watchdogArgs->threads) + 2)->threadType = PRINTER;

    return watchdogArgs;
}