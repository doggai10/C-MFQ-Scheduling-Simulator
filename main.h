#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  

#define MAX 100

typedef struct Process{
    int processId;
    int arrivalTime;
    int initQueue;
    int cycles;
    int cycleIndex;
    int burstTimes[MAX+1];
}Process;

typedef struct LinkedList{
    Process *task;
    struct LinkedList *next;
}Queue;

#endif