
#include "Process.h"

Process::Process(bool initPipe)
{
    numOfBoxes = 0;
    boxIndex = 0;

    if(initPipe)
    {
        InitPipe();
    }

    tasksComplete = false;

    processId = fork();

    if(processId == -1)
    {
        printf("Unable to fork new process");
        exit(EXIT_FAILURE);
    }
}

Process::Process(bool initPipe, int indexNum)
{
    numOfBoxes = NUMBER_OF_BOXES / NUMBER_OF_PHYS_PROCESSES;
    boxIndex = indexNum;

    if(initPipe)
    {
        InitPipe();
    }
    
    tasksComplete = false;

    processId = fork();

    if(processId == -1)
    {
        printf("Unable to fork new process");
        exit(EXIT_FAILURE);
    }
}

Process::~Process()
{
    exit(0);
}

void Process::InitPipe()
{
    if(pipe(pipefd) < 0)
    {
        printf("Unable to create pipe for process id: %d", processId);
        exit(EXIT_FAILURE);
    }
}