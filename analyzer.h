#ifndef ANALYZER_H
#define ANALYZER_H

#include "message_controller.h"

void *analyzerLoop();
struct CpuUsage *analyze(struct CpuReadData *, struct CpuTimeData *, int);
struct CpuTimeData *startingTimeData(int);

#include "analyzer.c"
#endif