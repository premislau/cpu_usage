#include "messege_controller.h"

static struct ReadDataMailbox* readDataMailbox; //TODO: critical section
static struct UsageMailbox* usageMailbox;
const int maxLen = 2;


enum SendingResult sendReadData(struct CpuReadData* readData){
    if(NULL == readDataMailbox){
        readDataMailbox = createReadDataMailbox(maxLen);
    }
    if(readDataMailbox->currentLen>=readDataMailbox->maxLen){
        return FULL;
    }   
    *(readDataMailbox->data + readDataMailbox->writeOffset)=readData;

    readDataMailbox->writeOffset = (readDataMailbox->writeOffset+1)%readDataMailbox->maxLen;
    ++readDataMailbox->currentLen;
    return SUCCESS;
}

enum SendingResult sendUsage(struct CpuUsage* usage){
    if(NULL == usageMailbox){
        usageMailbox = createUsageMailbox(maxLen);
    }
    if(usageMailbox->currentLen>=usageMailbox->maxLen){
        return FULL;
    }   
    *(usageMailbox->data + usageMailbox->writeOffset)=usage;

    usageMailbox->writeOffset = (usageMailbox->writeOffset+1)%usageMailbox->maxLen;
    ++usageMailbox->currentLen;
    return SUCCESS;
}


struct CpuReadData* receiveReadData(){
    if(NULL == readDataMailbox){
        readDataMailbox = createReadDataMailbox(maxLen);
    }
    if(readDataMailbox->currentLen<=0){
        return NULL;
    }
    // receiver gets a pointer to data; the pointer in mailbox can be overwritten; freeing memory is receiver's resposiblity
    struct CpuReadData* ret = *(readDataMailbox->data+readDataMailbox->readOffset);
    readDataMailbox->readOffset = (readDataMailbox->readOffset+1)%readDataMailbox->maxLen;
    --readDataMailbox->currentLen;
    return ret;
}

struct CpuUsage* receiveUsage(){
    if(NULL == usageMailbox){
        usageMailbox = createUsageMailbox(maxLen);
    }
    if(usageMailbox->currentLen<=0){
        return NULL;
    }
    // receiver gets a pointer to data; the pointer in mailbox can be overwritten; freeing memory is receiver's resposiblity
    struct CpuUsage* ret = *(usageMailbox->data+usageMailbox->readOffset);
    usageMailbox->readOffset = (usageMailbox->readOffset+1)%usageMailbox->maxLen;
    --usageMailbox->currentLen;
    return ret;
}

struct ReadDataMailbox* createReadDataMailbox(int maxLen){
    struct ReadDataMailbox* ret = malloc(sizeof(struct ReadDataMailbox));
    ret->maxLen=maxLen;
    ret->currentLen=0;
    ret->readOffset=0;
    ret->writeOffset=0;
    ret->data = malloc(maxLen*sizeof(struct CpuReadData));    
}

struct UsageMailbox* createUsageMailbox(int maxLen){
struct UsageMailbox* ret = malloc(sizeof(struct UsageMailbox));
    ret->maxLen=maxLen;
    ret->currentLen=0;
    ret->readOffset=0;
    ret->writeOffset=0;
    ret->data = malloc(maxLen*sizeof(struct CpuUsage));   
}

int getMaxLen(){
    return maxLen;
}