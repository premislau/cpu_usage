#include "cpu_data_structures.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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
    struct CpuReadData** data;    
};

struct UsageMailbox{
    int maxLen;
    int currentLen;
    int writeOffset;
    int readOffset;
    struct CpuUsage** data;    
};

enum SendingResult sendReadData(struct CpuReadData*);
enum SendingResult sendUsage(struct CpuUsage*);
struct CpuReadData* receiveReadData();
struct CpuUsage* receiveUsage();
struct ReadDataMailbox* createReadDataMailbox(int);
struct UsageMailbox* createUsageMailbox(int);
int getMaxLen();