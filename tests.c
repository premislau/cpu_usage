#include "program_logic.c"
#include <assert.h>

int main();
void testExtractDataFromRaw();
void testAnalyze();
struct CpuReadData *getMockCpuReadData(int, unsigned long long);
struct CpuUsage *getMockCpuUsage(int, double);
void testReadDataMailbox();
void testUsageMailbox();

int main()
{
    testReadDataMailbox();
    testUsageMailbox();
    // testExtractDataFromRaw();
    // testAnalyze();
    printf("All tests executed successfully.\n");
    return 0;
}

void testReadDataMailbox()
{
    enum SendingResult mr;
    struct CpuReadData *received;
    received = receiveReadData();
    assert(NULL == received);

    int maxLen = getMaxLen();
    int cpuCount = 2;
    for (int i = 0; i < maxLen; ++i)
    {
        mr = sendReadData(getMockCpuReadData(cpuCount, i));
        assert(SUCCESS == mr);
    }
    mr = sendReadData(getMockCpuReadData(cpuCount, 0));
    assert(FULL == mr);
    for (int i = 0; i < maxLen; ++i) // i values should be the same as in previous loop
    {
        received = receiveReadData();
        assert(i==received->guest);
        free(received);
    }
    received = receiveReadData();
    assert(NULL == received);
}

void testUsageMailbox()
{
    enum SendingResult mr;
    struct CpuUsage *received;
    received = receiveUsage();
    assert(NULL == received);

    int maxLen = getMaxLen();
    int cpuCount = 2;
    for (int i = 0; i < maxLen; ++i)
    {
        mr = sendUsage(getMockCpuUsage(cpuCount, (double) i));
        assert(SUCCESS == mr);
    }
    mr = sendUsage(getMockCpuUsage(cpuCount, (double) 0));
    assert(FULL == mr);
    for (int i = 0; i < maxLen; ++i) // i values should be the same as in previous loop
    {
        received = receiveUsage();
        assert((double)i==received->usage);
        free(received);
    }
    received = receiveUsage();
    assert(NULL == received);
}

void testAnalyze()
{
    int cpuCount = 2;
    struct CpuTimeData *prevTimeData = startingTimeData(cpuCount);
    struct CpuReadData *readData;
    struct CpuUsage *usage;
    for (unsigned long long j = 100; j < 1000; j += 50)
    {
        readData = getMockCpuReadData(cpuCount, j);
        usage = analyze(readData, prevTimeData, cpuCount);
        for (int i = 0; i < cpuCount; ++i)
        {
            unsigned long long expectedIdlde = 2 * j;
            unsigned long long expectedNonIdlde = 6 * j;
            unsigned long long expectedTotal = 8 * j;
            assert(expectedIdlde == (prevTimeData + i)->Idle);
            assert(expectedNonIdlde == (prevTimeData + i)->NonIdle);
            assert(expectedTotal == (prevTimeData + i)->Total);

            double expectedUsage = 75.0;
            assert(expectedUsage == (usage + i)->usage);
        }
        free(readData);
        free(usage);
    }
}

struct CpuReadData *getMockCpuReadData(int cpuCount, unsigned long long fillValue)
{
    struct CpuReadData *ret = (struct CpuReadData *)malloc(cpuCount * sizeof(struct CpuReadData));
    for (int i = 0; i < cpuCount; ++i)
    {
        (ret + i)->user = fillValue;
        (ret + i)->nice = fillValue;
        (ret + i)->system = fillValue;
        (ret + i)->idle = fillValue;
        (ret + i)->iowait = fillValue;
        (ret + i)->irq = fillValue;
        (ret + i)->softirq = fillValue;
        (ret + i)->steal = fillValue;
        (ret + i)->guest = fillValue;
        (ret + i)->guestnice = fillValue;
        char *cpu_id = malloc(9 * sizeof(char));
        sprintf(cpu_id, "cpu%d", i);
        (ret+i)->cpu_id = cpu_id;
    }
    return ret;
}

struct CpuUsage *getMockCpuUsage(int cpuCount, double usage)
{
    struct CpuUsage *ret = (struct CpuUsage *)malloc(cpuCount * sizeof(struct CpuUsage));
    for (int i = 0; i < cpuCount; ++i)
    {
        (ret+i)->usage=usage;
        char *cpu_id = malloc(9 * sizeof(char));
        sprintf(cpu_id, "cpu%d", i);
        (ret+i)->cpu_id = cpu_id;
    }
    return ret;
}

void testExtractDataFromRaw()
{
    char *rawData = "cpu  46681 0 9362 108948 10658 0 606 0 0 0\n\
cpu0 22494 0 4253 55891 5468 0 296 0 0 0\n\
cpu1 24186 0 5109 53057 5189 0 310 0 0 0\n\
intr 560979 29 1150 0 0 0 0 0 0 0 0 0 0 1422 0 0 1013 0 0 3430 4913 17085 23806 27 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n\
ctxt 1819265\n\
btime 1670677955\n\
processes 3191\n\
procs_running 1\n\
procs_blocked 0\n\
softirq 416281 4 176423 175 5574 22987 0 1096 89499 0 120523";
    int cpuCount = 2;
    struct CpuReadData *readData = extractDataFromRaw(rawData, cpuCount);

    int i = 0;
    assert((readData + i)->user == 22494);
    assert((readData + i)->nice == 0);
    assert((readData + i)->system == 4253);
    assert((readData + i)->idle == 55891);
    assert((readData + i)->iowait == 5468);
    assert((readData + i)->irq == 0);
    assert((readData + i)->softirq == 296);
    assert((readData + i)->steal == 0);
    assert((readData + i)->guest == 0);
    assert((readData + i)->guestnice == 0);

    ++i;
    assert((readData + i)->user == 24186);
    assert((readData + i)->nice == 0);
    assert((readData + i)->system == 5109);
    assert((readData + i)->idle == 53057);
    assert((readData + i)->iowait == 5189);
    assert((readData + i)->irq == 0);
    assert((readData + i)->softirq == 310);
    assert((readData + i)->steal == 0);
    assert((readData + i)->guest == 0);
    assert((readData + i)->guestnice == 0);
}
