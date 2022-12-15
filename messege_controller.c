#include "messege_controller.h"

static struct ReadDataMailbox *readDataMailbox; // TODO: critical section
static struct UsageMailbox *usageMailbox;
static struct LogMailbox *logMailbox;
int cpuCount = 0; // cpuCount is initialised to non-zero value by reader
const int maxLen = 2;
useconds_t lockWaitingTime = 10;

enum SendingResult sendReadData(struct CpuReadData *readData)
{
    while (0 != pthread_mutex_lock(&(readDataMailbox->lock))) // 0 means success
    {
        usleep(lockWaitingTime);
    }
    if (readDataMailbox->currentLen >= readDataMailbox->maxLen)
    {
        pthread_mutex_unlock(&(readDataMailbox->lock));
        return FULL;
    }
    *(readDataMailbox->data + readDataMailbox->writeOffset) = readData;

    readDataMailbox->writeOffset = (readDataMailbox->writeOffset + 1) % readDataMailbox->maxLen;
    ++readDataMailbox->currentLen;
    pthread_mutex_unlock(&(readDataMailbox->lock));
    return SUCCESS;
}

enum SendingResult sendUsage(struct CpuUsage *usage)
{
    while (0 != pthread_mutex_lock(&(usageMailbox->lock))) // 0 means success
    {
        usleep(lockWaitingTime);
    }
    if (usageMailbox->currentLen >= usageMailbox->maxLen)
    {
        pthread_mutex_unlock(&(usageMailbox->lock));
        return FULL;
    }
    *(usageMailbox->data + usageMailbox->writeOffset) = usage;

    usageMailbox->writeOffset = (usageMailbox->writeOffset + 1) % usageMailbox->maxLen;
    ++usageMailbox->currentLen;
    pthread_mutex_unlock(&(usageMailbox->lock));
    return SUCCESS;
}

enum SendingResult sendLog(char *log)
{
    while (0 != pthread_mutex_lock(&(logMailbox->lock))) // 0 means success
    {
        usleep(lockWaitingTime);
    }
    if (logMailbox->currentLen >= logMailbox->maxLen)
    {
        pthread_mutex_unlock(&(logMailbox->lock));
        return FULL;
    }
    *(logMailbox->data + logMailbox->writeOffset) = log;

    logMailbox->writeOffset = (logMailbox->writeOffset + 1) % logMailbox->maxLen;
    ++logMailbox->currentLen;
    pthread_mutex_unlock(&(logMailbox->lock));
    return SUCCESS;
}

struct CpuReadData *receiveReadData()
{
    while (0 != pthread_mutex_lock(&(readDataMailbox->lock))) // 0 means success
    {
        usleep(lockWaitingTime);
    }
    if (readDataMailbox->currentLen <= 0)
    {
        pthread_mutex_unlock(&(readDataMailbox->lock));
        return NULL;
    }
    // receiver gets a pointer to data; the pointer in mailbox can be overwritten; freeing memory is receiver's resposiblity
    struct CpuReadData *ret = *(readDataMailbox->data + readDataMailbox->readOffset);
    readDataMailbox->readOffset = (readDataMailbox->readOffset + 1) % readDataMailbox->maxLen;
    --readDataMailbox->currentLen;
    pthread_mutex_unlock(&(readDataMailbox->lock));
    return ret;
}

struct CpuUsage *receiveUsage()
{
    while (0 != pthread_mutex_lock(&(usageMailbox->lock))) // 0 means usccess
    {
        usleep(lockWaitingTime);
    }
    if (usageMailbox->currentLen <= 0)
    {
        pthread_mutex_unlock(&(usageMailbox->lock));
        return NULL;
    }
    // receiver gets a pointer to data; the pointer in mailbox can be overwritten; freeing memory is receiver's resposiblity
    struct CpuUsage *ret = *(usageMailbox->data + usageMailbox->readOffset);
    usageMailbox->readOffset = (usageMailbox->readOffset + 1) % usageMailbox->maxLen;
    --usageMailbox->currentLen;
    pthread_mutex_unlock(&(usageMailbox->lock));
    return ret;
}

char* receiveLog()
{
    while (0 != pthread_mutex_lock(&(logMailbox->lock))) // 0 means usccess
    {
        usleep(lockWaitingTime);
    }
    if (logMailbox->currentLen <= 0)
    {
        pthread_mutex_unlock(&(usageMailbox->lock));
        return NULL;
    }
    // receiver gets a pointer to data; the pointer in mailbox can be overwritten; freeing memory is receiver's resposiblity
    char* ret = *(logMailbox->data + logMailbox->readOffset);
    logMailbox->readOffset = (logMailbox->readOffset + 1) % logMailbox->maxLen;
    --logMailbox->currentLen;
    pthread_mutex_unlock(&(logMailbox->lock));
    return ret;
}

void initReadDataMailbox(int maxLen)
{
    readDataMailbox = createReadDataMailbox(maxLen);
}

void initUsageMailbox(int maxLen)
{
    usageMailbox = createUsageMailbox(maxLen);
}

void initLogMailbox(int maxLen)
{
    logMailbox = createLogMailbox(maxLen);
}

struct ReadDataMailbox *createReadDataMailbox(int maxLen)
{
    struct ReadDataMailbox *ret = malloc(sizeof(struct ReadDataMailbox));
    ret->maxLen = maxLen;
    ret->currentLen = 0;
    ret->readOffset = 0;
    ret->writeOffset = 0;
    if (0 != pthread_mutex_init(&(ret->lock), NULL))
    {
        printf("Error during creation of mutex.\n");
    }
    ret->lock;
    ret->data = malloc(maxLen * sizeof(struct CpuReadData));
}

struct UsageMailbox *createUsageMailbox(int maxLen)
{
    struct UsageMailbox *ret = malloc(sizeof(struct UsageMailbox));
    ret->maxLen = maxLen;
    ret->currentLen = 0;
    ret->readOffset = 0;
    ret->writeOffset = 0;
    if (0 != pthread_mutex_init(&(ret->lock), NULL))
    {
        printf("Error during creation of mutex.\n");
    }
    ret->data = malloc(maxLen * sizeof(struct CpuUsage));
}

struct LogMailbox *createLogMailbox(int maxLen)
{
    struct LogMailbox *ret = malloc(sizeof(struct LogMailbox));
    ret->maxLen = maxLen;
    ret->currentLen = 0;
    ret->readOffset = 0;
    ret->writeOffset = 0;
    if (0 != pthread_mutex_init(&(ret->lock), NULL))
    {
        printf("Error during creation of mutex.\n");
    }
    ret->data = malloc(maxLen * sizeof(char *));
}