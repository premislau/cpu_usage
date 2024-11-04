#ifndef message_CONTROLLER_H
#define message_CONTROLLER_H

#include "cpu_data_structures.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

int cpuCount = 0; // cpuCount is initialised to non-zero value by reader

enum SendingResult
{
    SUCCESS,
    FULL
};

enum LogType
{
    NONE_LOG,
    PROGRAM_STARTED,
    TERMINATION_BY_WATCHDOG
};

struct Log
{
    enum LogType logType;
    int detail; // eg. error code
};

enum ThreadType
{
    NONE_THREAD,
    READER,
    ANALYZER,
    PRINTER
};

struct LogMailbox
{
    int maxLen;
    int currentLen;
    int writeOffset;
    int readOffset;
    pthread_mutex_t lock;
    struct Log *data;
};

struct ReadDataMailbox
{
    int maxLen;
    int currentLen;
    int writeOffset;
    int readOffset;
    pthread_mutex_t lock;
    struct CpuReadData **data;
};

struct UsageMailbox
{
    int maxLen;
    int currentLen;
    int writeOffset;
    int readOffset;
    pthread_mutex_t lock;
    struct CpuUsage **data;
};

struct ActivenessMailbox
{
    int maxLen;
    int currentLen;
    int writeOffset;
    int readOffset;
    pthread_mutex_t lock;
    enum ThreadType *data;
};

enum SendingResult sendReadData(struct CpuReadData *);
enum SendingResult sendUsage(struct CpuUsage *);
enum SendingResult sendActiveness(enum ThreadType);
enum SendingResult sendLog(struct Log);
struct CpuReadData *receiveReadData();
struct CpuUsage *receiveUsage();
enum ThreadType receiveActiveness();
struct Log receiveLog();
void initReadDataMailbox(int);
void initUsageMailbox(int);
void initActivenessMailbox(int);
void initLogMailbox(int);
struct ReadDataMailbox *createReadDataMailbox(int);
struct UsageMailbox *createUsageMailbox(int);
struct ActivenessMailbox *createActivenessMailbox(int);
struct LogMailbox *createLogMailbox(int);


#include "message_controller.c"
#endif