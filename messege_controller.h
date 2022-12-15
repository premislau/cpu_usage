#include "cpu_data_structures.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


enum SendingResult{
    SUCCESS,
    NO_RECEIVER,
    FULL
};

struct ReadDataMailbox{
    int maxLen;
    int currentLen;
    int writeOffset;
    int readOffset;
    pthread_mutex_t lock;
    struct CpuReadData** data;    
};

struct UsageMailbox{
    int maxLen;
    int currentLen;
    int writeOffset;
    int readOffset;
    pthread_mutex_t lock;
    struct CpuUsage** data;    
};


struct LogMailbox{
    int maxLen;
    int currentLen;
    int writeOffset;
    int readOffset;
    pthread_mutex_t lock;
    char** data;
};

enum SendingResult sendReadData(struct CpuReadData*);
enum SendingResult sendUsage(struct CpuUsage*);
struct CpuReadData* receiveReadData();
struct CpuUsage* receiveUsage();
void initReadDataMailbox(int);
void initUsageMailbox(int);
struct ReadDataMailbox* createReadDataMailbox(int);
struct UsageMailbox* createUsageMailbox(int);
struct LogMailbox* createLogMailbox(int);