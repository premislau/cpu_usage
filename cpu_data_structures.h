struct CpuReadData
{
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guestnice;
    char *cpu_id;
};

struct CpuTimeData
{
    unsigned long long Idle;
    unsigned long long NonIdle;
    unsigned long long Total;
    char *cpu_id;
};

struct CpuUsage
{
    char *cpu_id;
    double usage;
};