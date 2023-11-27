
#include "Process.h"

Process::Process(bool initPipe)
{
    processId = fork();

    if(processId == -1)
    {
        perror("Unable to fork new process");
        exit(EXIT_FAILURE);
    }

    if(initPipe)
    {
        InitPipe();
    }

    tasksComplete = false;
}

Process::Process(bool initPipe, int indexNum)
{
    processId = fork();

    if(processId == -1)
    {
        perror("Unable to fork new process");
        exit(EXIT_FAILURE);
    }

    numOfBoxes = NUMBER_OF_BOXES / NUMBER_OF_PHYS_PROCESSES;
    boxIndex = indexNum;

    if(initPipe)
    {
        InitPipe();
    }
    
    tasksComplete = false;
}

Process::~Process()
{
    exit(0);
}

void Process::InitPipe()
{
    if(pipe(pipefd) < 0)
    {
        perror("Unable to create pipe for process id: ");
        std::cout << processId;
        exit(EXIT_FAILURE);
    }
}