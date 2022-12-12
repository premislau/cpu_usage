#include "program_logic.c"
#include <unistd.h>

int main();

int main()
{
    int cpuCount = 0;
    useconds_t sleepTime= 1000000;
    struct CpuReadData *readData = readProcStat(&cpuCount);
    struct CpuTimeData *prevTimeData = startingTimeData(cpuCount);
    struct CpuUsage *usage = analyze(readData, prevTimeData, cpuCount);
    print(usage, cpuCount);
    usleep(sleepTime);
    for (int i = 0; i < 10; ++i)
    {
        free(readData);
        readData = readProcStat(&cpuCount);
        free(usage);
        usage = analyze(readData, prevTimeData, cpuCount);
        print(usage, cpuCount);
        usleep(sleepTime);
    }

    return 0;
}

