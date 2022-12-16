struct CpuReadData
{
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guestnice;
    int index;
};

struct CpuTimeData
{
    unsigned long long Idle;
    unsigned long long NonIdle;
    unsigned long long Total;
    int index;
};

struct CpuUsage
{
    double usage;
    int index;
};
