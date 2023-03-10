//#include "program_logic.h"


void *readerLoop()
{
    enum SendingResult sr;
    while (1)
    {
        struct CpuReadData *readData = readProcStat(&cpuCount);
        sr = sendReadData(readData);
        while (FULL == sr)
        {
            usleep(dataWaitingTime);
            sr = sendReadData(readData);
        }
        sr = sendActiveness(READER);
            while (SUCCESS != sr)
            {
                usleep(dataWaitingTime);
                sr = sendActiveness(READER);
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
            while (SUCCESS != sr)
            {
                usleep(dataWaitingTime);
                sr = sendActiveness(ANALYZER);
            }
        usleep(nextIterationWaitingTime);
    }
}

void *printerLoop()
{
    enum SendingResult sr;
    struct CpuUsage *usage;
    while (1)
    {
        usage = receiveUsage();
        while (NULL == usage)
        {
            usleep(dataWaitingTime);
            usage = receiveUsage();
        }
        print(usage, cpuCount);
        free(usage);
        
            sr = sendActiveness(PRINTER);
            while (SUCCESS != sr)
            {
                usleep(dataWaitingTime);
                sr = sendActiveness(PRINTER);
            }
        
        usleep(nextIterationWaitingTime);
    }
}

void *watchdogLoop(void *args)
{
    struct Log log;
    log.logType = PROGRAM_STARTED;
    sendLog(log);
    struct WatchdogArgs *data = (struct WatchdogArgs *)args;
    int *activeness = malloc(data->numberOfThreads * sizeof(int)); // bolean activeness of corresponding thread
    for (int i = 0; i < data->numberOfThreads; ++i)
    {
        *(activeness + i) = 0;
    }
    enum ThreadType received;
    usleep(watchdogNextIterationWaitingTime);
    while (1)
    {
        while ((received = receiveActiveness()) != NONE_THREAD)
        {
            for (int i = 0; i < data->numberOfThreads; ++i)
            {
                if (received == ((data->threads + i)->threadType))
                {
                    *(activeness + i) = 1;
                    break;
                }
            }
        }
        for (int i = 0; i < data->numberOfThreads; ++i)
        {
            if (0 == *(activeness + i))
            {
                printf("One of the threads is not active!\n");
                printf("Program will be terminated!\n");
                for (int i = 0; i < data->numberOfThreads; ++i)
                {
                    pthread_cancel((data->threads + i)->threadId);
                }
                log.logType = TERMINATION_BY_WATCHDOG;
                sendLog(log);
                void *retval = NULL;
                pthread_exit(retval);
            }
        }
        for (int i = 0; i < data->numberOfThreads; ++i)
        {
            *(activeness + i) = 0;
        }
        usleep(watchdogNextIterationWaitingTime);
    }
}

void *loggerLoop(void *args)
{
    char *loggerFile = (char *)args;
    struct Log received;
    while (1)
    {
        received = receiveLog();
        while (NONE_LOG != received.logType)
        {
            saveLog(loggerFile, received);
            if(TERMINATION_BY_WATCHDOG == received.logType){
                void *retval = NULL;
                pthread_exit(retval);
            }
            received = receiveLog();
        }
        usleep(loggerNextIterationWaitingTime);
    }
}

struct CpuReadData *readProcStat(int *cpuCount)
{
    char *rawData = readRawData();
    if (0 == *cpuCount)
    { // cpuCount is set to non-zero value only once
        *cpuCount = extractCpuCount(rawData);
    }
    struct CpuReadData *ret = extractDataFromRaw(rawData, *cpuCount);
    free(rawData);
    return ret;
}

struct CpuReadData *extractDataFromRaw(char *rawData, int cpuCount)
{
    if (NULL == cpuNameStore)
    {
        initCpuNameStore(cpuCount);
    }
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
        sscanf(rawData + offset, temp,
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
    for (int i = 0; i < cpuCount; ++i)
    {
        char *cpuName = getCpuName((data + i)->index);
        printf("%s: %f%%, ", cpuName, (data + i)->usage);
    }
    printf("\n");
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

void saveLog(char *loggerFile, struct Log log)
{
    FILE *file = fopen(loggerFile, "a");
    if (NULL == file)
    {
        printf("file cannot be opened\n");
    }
    char* strLog = logToString(log);
    time_t now = time(0);
    fprintf(file, "%s; %s", strLog, asctime(gmtime(&now)));
    fclose(file);
}

char *logToString(struct Log log)
{
    char* ret = malloc(40*sizeof(char));
    switch (log.logType)
    {
    case PROGRAM_STARTED:
        sprintf(ret, "Program has been started");
        
        break;
    case TERMINATION_BY_WATCHDOG:
        sprintf(ret, "Program is terminated by watchdog");
        break;
    default:
        sprintf(ret, "Unknown log");
        break;
    }
    return ret;
}

char *readRawData()
{
    FILE *file = fopen("/proc/stat", "r");
    if (NULL == file)
    {
        printf("file cannot be opened\n");
    }
    int maxLen = 0x10000;
    char *rawData = malloc(maxLen * sizeof(char));
    fread(rawData, sizeof(char), maxLen, file);
    fclose(file);
    return rawData;
}

int extractCpuCount(char *rawData)
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


void initCpuNameStore(int cpuCount)
{
    cpuNameStore = malloc(cpuCount * sizeof(char *));
    int numberOfDigits = getNumberOfDigits(cpuCount);
    int nameChars = 4 + numberOfDigits;
    for (int i = 0; i < cpuCount; ++i)
    {
        char *name = malloc(nameChars * sizeof(char));
        sprintf(name, "cpu%d", i); // matches naming in /proc/stat in the year 2022
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
