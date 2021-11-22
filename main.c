#include "main.h"

FILE *file;
int totalProcess, startTime, finishTime, globalTime = 0, finishJob = 0, timeQuantum = 0;
bool waitCheck = false;

Process *totalQueue[MAX + 1]; // before allocate ready Queue;
Queue *readyQueue[4];         //ready Queue;
Process *sleepQueue[MAX + 1]; // sleep Queue;
Process *nowProcess;          // now process;
int resultTable[MAX + 1][2];  //saving sceduling result table;

void initializeQueue();
bool allocateProcess();
void simulation();
void processArrive();
void ioWakeUp();
bool scheduling();
bool cpuBurst();
void ioBurst();
void ganttChart();
void printResult();
void allocateReadyQueue(Process *task, int queNumber);
void deAllocateReadyQueue(Process *task, int queNumber);

int main(int argc, char *argv[])
{
    file = fopen("./input.txt", "r");
    if (file == NULL)
    {
        printf("Couldn't find input file\n");
    }
    else if (fscanf(file, "%d", &totalProcess) > 0)
    {

        if (totalProcess > 0)
        {
            initializeQueue();
            bool inputCheck = allocateProcess();
            fclose(file);
            if (inputCheck)
            {
                simulation();
                printResult();
            }
            else
            {
                printf("input file doesn't follow instructions\n");
            }
        }
        else if (totalProcess == 0)
        {
            printf("there isn't job to schedule\n");
        }
        else
        {
            printf("input file doesn't follow instructions\n");
        }
    }
    else
    {
        printf("input file doesn't follow instructions\n");
    }
    return 0;
}

void initializeQueue()
{

    for (int i = 0; i < 4; i++)
    {
        readyQueue[i] = malloc(sizeof(Queue));
    }
}

bool allocateProcess()
{
    int processId, arrivalTime, initQueue, cycles, total = 0, value = 0;
    for (int i = 0; i < totalProcess; i++)
    {
        if (fscanf(file, "%d %d %d %d", &processId, &arrivalTime, &initQueue, &cycles) < 0)
            return false;
        if (cycles == 0)
            return false;
        // make new Process and allocate to totalQueue
        Process *newProcess = (Process *)malloc(sizeof(Process));
        newProcess->processId = processId;
        newProcess->arrivalTime = arrivalTime;
        newProcess->initQueue = initQueue;
        newProcess->cycles = cycles;
        newProcess->cycleIndex = 0;
        int size = (cycles * 2) - 1;
        for (int j = 0; j < MAX+1; j++)
        {
            newProcess->burstTimes[j] = 0;
            if (j < size)
            {
                if(fscanf(file, "%d", &value)<0){
                    return false;
                }
                newProcess->burstTimes[j] = value;
                total += value;
            }
        }
        totalQueue[processId] = newProcess;
        // save totalBurst time to result Table
        resultTable[processId][1] = total * -1;
        total = 0;
    }
    return true;
}

void simulation()
{
    printf("----------- Gantt Chart -----------\n");
    while (finishJob < totalProcess)
    {
        //process arrive
        processArrive();
        //io wakeup
        ioWakeUp();
        bool scheduleResult = scheduling();
        if (scheduleResult)
        {
            if (waitCheck == true)
            {
                //waiting time print
                finishTime = globalTime;
                printf("Wait %d ~ %d\n", startTime, finishTime);
                waitCheck = false;
            }
            startTime = globalTime;
            while (timeQuantum > 0)
            {
                //ioBurst
                ioBurst();
                //cpuBUrst
                bool done = cpuBurst();
                globalTime++;
                timeQuantum--;
                //job Done
                if (done)
                {
                    finishTime = globalTime;
                    ganttChart();
                    break;
                }
                // couldn't finish job during time Quantum
                else
                {
                    // if timeQuantum is 0, reAllocate process to another readyQueue
                    if (timeQuantum == 0)
                    {
                        finishTime = globalTime;
                        int queNumber = nowProcess->initQueue;
                        ganttChart();
                        if (nowProcess->initQueue < 3)
                        {
                            nowProcess->initQueue++;
                        }
                        deAllocateReadyQueue(nowProcess, queNumber);
                        allocateReadyQueue(nowProcess, nowProcess->initQueue);
                        nowProcess = NULL;
                        break;
                    }
                }
                //process arrive
                processArrive();
                //io wakeup
                ioWakeUp();
            }
        }
        else
        {
            //waiting check
            if (waitCheck == false)
            {
                waitCheck = true;
                startTime = globalTime;
            }
            //ioBurst
            ioBurst();
            globalTime++;
        }
    }
}

void processArrive()
{
    for (int i = 0; i < MAX+1; i++)
    {
        Process *task = totalQueue[i];
        if (task == NULL)
            continue;
        if (task->arrivalTime == globalTime)
        {
            allocateReadyQueue(task, task->initQueue);
            totalQueue[i] = NULL;
        }
    }
}

void ioWakeUp()
{
    for (int i = 0; i < MAX+1; i++)
    {
        Process *task = sleepQueue[i];
        if (task == NULL)
            continue;
        if (task->burstTimes[task->cycleIndex] == 0)
        {
            sleepQueue[i]->cycleIndex++;
            int promotion = sleepQueue[i]->initQueue;
            //readyQueue3 always stay in readyQueue3;
            if (promotion != 3)
                sleepQueue[i]->initQueue--;
            allocateReadyQueue(sleepQueue[i], sleepQueue[i]->initQueue);
            sleepQueue[i] = NULL;
        }
    }
}

void allocateReadyQueue(Process *task, int queNumber)
{
    Queue *head = readyQueue[queNumber];
    Queue *insert = (Queue *)malloc(sizeof(Queue));
    // if there isn't process in readyQueue
    if (head->task == NULL)
    {
        head->task = task;
        head->next = NULL;
    }
    // if there are process in readyQueue and find last process
    else
    {
        while (head->next != NULL)
        {
            head = head->next;
        }
        head->next = insert;
        insert->task = task;
        insert->next = NULL;
    }
}

void deAllocateReadyQueue(Process *task, int queNumber)
{
    Queue *head = readyQueue[queNumber];
    Queue *prev = head;
    // find same process;
    while (head->task->processId != task->processId)
    {
        prev = head;
        head = head->next;
    }
    // if prev equal head
    if (prev->task->processId == head->task->processId)
    {
        //delete prev
        if (head->next == NULL)
        {
            prev->task = NULL;
            prev->next = NULL;
        }
        // delete head
        else
        {
            prev->task = head->next->task;
            prev->next = head->next->next;
        }
    }
    // delete head process
    else
    {
        prev->next = head->next;
        head->task = NULL;
    }
}

bool scheduling()
{

    Queue *head;
    for (int i = 0; i < 4; i++)
    {
        head = readyQueue[i];
        if (head->task == NULL)
            continue;
        else
        {
            // set appropriate timeQuantum
            if (i == 0)
                timeQuantum = 1;
            else if (i == 1)
                timeQuantum = 2;
            else if (i == 2)
                timeQuantum = 4;
            else
            {
                //readyQueue 3 fcfs, so set timeQuantum as remaining time
                int cycleIndex = head->task->cycleIndex;
                timeQuantum = head->task->burstTimes[cycleIndex];
            }
            nowProcess = head->task;
            return true;
        }
    }
    return false;
}

bool cpuBurst()
{
    int jobRemain = --nowProcess->burstTimes[nowProcess->cycleIndex];
    int pId = nowProcess->processId;
    if (jobRemain == 0)
    {
        nowProcess->cycleIndex++;
        int totalCycle = (nowProcess->cycles * 2) - 1;
        // if there isn' no remaiing burst, then delete process
        if (totalCycle == nowProcess->cycleIndex)
        {
            int turnArondTime = globalTime - nowProcess->arrivalTime + 1;
            resultTable[pId][0] = turnArondTime;
            resultTable[pId][1] += turnArondTime;
            finishJob++;
            deAllocateReadyQueue(nowProcess, nowProcess->initQueue);
        }
        // allocate process to sleep Queue
        else
        {
            deAllocateReadyQueue(nowProcess, nowProcess->initQueue);
            sleepQueue[pId] = nowProcess;
        }
        return true;
    }
    else
    {
        return false;
    }
}

void ioBurst()
{
    //every process sleep queue do ioBurst
    for (int i = 0; i < MAX+1; i++)
    {
        if (sleepQueue[i] != NULL)
        {
            sleepQueue[i]->burstTimes[sleepQueue[i]->cycleIndex]--;
        }
    }
}

void ganttChart()
{
    printf("time: %d ~ %d, processId: %d, Queue %d\n", startTime, finishTime, nowProcess->processId, nowProcess->initQueue);
}

void printResult()
{
    printf("----------- Gantt Chart End-----------\n\n");
    printf("--------------------  Result Table  --------------------\n");
    double turnaroundTimeAVG = 0, waitingTimeAVG = 0;
    printf("| Process ID   || Turnaround Time   || Waiting Time    |\n");
    for (int i = 1; i < MAX+1; i++)
    {
        if(resultTable[i][0]==0 && resultTable[i][1]==0) break;
        printf("|-----%3d -----|", i + 1);
        printf("|------- %3d -------|", resultTable[i][0]);
        printf("|------ %3d ------|\n", resultTable[i][1]);
        turnaroundTimeAVG += resultTable[i][0];
        waitingTimeAVG += resultTable[i][1];
    }
    turnaroundTimeAVG /= totalProcess;
    waitingTimeAVG /= totalProcess;
    printf("turnaround time average: %.2fs\n", turnaroundTimeAVG);
    printf("waiting time average: %.2fs\n", waitingTimeAVG);
    printf("------------------  Result Table End  ------------------\n");
}