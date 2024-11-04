#ifndef PROGRAM_LOGIC_H
#define PROGRAM_LOGIC_H

#include "message_controller.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

const useconds_t dataWaitingTime = 50000;
const useconds_t nextIterationWaitingTime = 950000;
const useconds_t watchdogNextIterationWaitingTime = 2000000;
const useconds_t loggerNextIterationWaitingTime = 300000;
char **cpuNameStore = NULL;

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
void *loggerLoop(void*);
void *watchdogLoop(void*);

struct CpuReadData *readProcStat(int *);
struct CpuUsage *analyze(struct CpuReadData *, struct CpuTimeData *, int);
void print(struct CpuUsage *, int);
int extractCpuCount(char *);
char *readRawData();
struct CpuReadData *extractDataFromRaw(char *, int);
void *analyzerLoop();
struct CpuUsage *analyze(struct CpuReadData *, struct CpuTimeData *, int);
struct CpuTimeData *startingTimeData(int);

void initCpuNameStore(int);
char *getCpuName(int);
int getNumberOfDigits(int);
void saveLog(char*, struct Log);
char *logToString(struct Log);


#include "program_logic.c"
#endif