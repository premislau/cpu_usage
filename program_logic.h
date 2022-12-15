#include "messege_controller.c"

void *readerLoop();
void *analyzerLoop();
void *printerLoop();
void *loggerLoop();

struct CpuReadData *readProcStat(int *);
struct CpuUsage *analyze(struct CpuReadData *, struct CpuTimeData *, int);
void print(struct CpuUsage *, int);
int extraxtCpuCount(char *);
char *readRawData();
struct CpuReadData *extractDataFromRaw(char *, int);
struct CpuTimeData *startingTimeData(int);