//#include "message_controller.h"

static struct ReadDataMailbox *readDataMailbox;
static struct UsageMailbox *usageMailbox;
static struct ActivenessMailbox *activenessMailbox;
static struct LogMailbox *logMailbox;

enum SendingResult sendReadData(struct CpuReadData *readData)
{
    pthread_mutex_lock(&(readDataMailbox->lock));
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
    pthread_mutex_lock(&(usageMailbox->lock));
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

enum SendingResult sendActiveness(enum ThreadType activeness){
    pthread_mutex_lock(&(activenessMailbox->lock));
    if (activenessMailbox->currentLen >= activenessMailbox->maxLen)
    {
        pthread_mutex_unlock(&(activenessMailbox->lock));
        return FULL;
    }
    *(activenessMailbox->data + activenessMailbox->writeOffset) = activeness;

    activenessMailbox->writeOffset = (activenessMailbox->writeOffset + 1) % activenessMailbox->maxLen;
    ++activenessMailbox->currentLen;
    pthread_mutex_unlock(&(activenessMailbox->lock));
    return SUCCESS;
}

enum SendingResult sendLog(struct Log log)
{
    pthread_mutex_lock(&(logMailbox->lock));
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
    pthread_mutex_lock(&(readDataMailbox->lock));
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
    pthread_mutex_lock(&(usageMailbox->lock));
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

enum ThreadType receiveActiveness()
{
    pthread_mutex_lock(&(activenessMailbox->lock));
    if (activenessMailbox->currentLen <= 0)
    {
        pthread_mutex_unlock(&(activenessMailbox->lock));
        return NONE_THREAD;
    }
    //receiver gets enum value, there is no need for repetitive memory freeing
    enum ThreadType ret = *(activenessMailbox->data + activenessMailbox->readOffset);
    activenessMailbox->readOffset = (activenessMailbox->readOffset + 1) % activenessMailbox->maxLen;
    --activenessMailbox->currentLen;
    pthread_mutex_unlock(&(activenessMailbox->lock));
    return ret;
}

struct Log receiveLog()
{
    struct Log ret;
    pthread_mutex_lock(&(logMailbox->lock));
    if (logMailbox->currentLen <= 0)
    {
        pthread_mutex_unlock(&(logMailbox->lock));
        ret.logType=NONE_LOG;
        ret.detail=0;
        return ret;
    }
    // receiver gets a struct, there is no need for memory freeing
    ret = *(logMailbox->data + logMailbox->readOffset);
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

void initActivenessMailbox(int maxLen)
{
    activenessMailbox = createActivenessMailbox(maxLen);
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
    ret->data = malloc(maxLen * sizeof(struct CpuReadData));
    return ret;
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
    return ret;
}

struct ActivenessMailbox *createActivenessMailbox(int maxLen)
{
    struct ActivenessMailbox *ret = malloc(sizeof(struct ActivenessMailbox));
    ret->maxLen = maxLen;
    ret->currentLen = 0;
    ret->readOffset = 0;
    ret->writeOffset = 0;
    if (0 != pthread_mutex_init(&(ret->lock), NULL))
    {
        printf("Error during creation of mutex.\n");
    }
    ret->data = malloc(maxLen * sizeof(enum ThreadType));
    return ret;
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
    ret->data = malloc(maxLen * sizeof(struct Log));
    return ret;
}