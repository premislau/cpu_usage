#include "messege_controller.c"

int watchdogActive = 0; //boolean, 0 is default value

struct WatchedThread{
    pthread_t threadId;
    enum ThreadType threadType;
};

struct WatchdogArgs{
    int numberOfThreads;
    struct WatchedThread* threads;
};

void *readerLoop();
void *analyzerLoop();
void *printerLoop();
void *loggerLoop();

struct CpuReadData *readProcStat(int *);
struct CpuUsage *analyze(struct CpuReadData *, struct CpuTimeData *, int);
void print(struct CpuUsage *, int);
int extractCpuCount(char *);
char *readRawData();
struct CpuReadData *extractDataFromRaw(char *, int);
struct CpuTimeData *startingTimeData(int);

void initCpuNameStore(int);
char *getCpuName(int);
int getNumberOfDigits(int);
void saveLog(char*, struct Log);
char *logToString(struct Log);