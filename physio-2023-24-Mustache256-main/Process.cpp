
#include "Process.h"

Process::Process(int pipeI)
{
    //Initialising values
    numOfBoxes = 0;
    boxIndex = 0;
    pipeIndex = pipeI;

    tasksComplete = false;

    //Forking new process and checking that fork was successful
    processId = fork();

    if(processId == -1)
    {
        printf("Unable to fork new process");
        exit(EXIT_FAILURE);
    }
}

Process::Process(int pipeI, int boxIndexNum)
{
    //Initialising values
    numOfBoxes = NUMBER_OF_BOXES / NUMBER_OF_PHYS_PROCESSES;
    boxIndex = boxIndexNum;
    pipeIndex = pipeI;
    
    tasksComplete = false;

    //Forking new process and checking that fork was successful
    processId = fork();

    if(processId == -1)
    {
        printf("Unable to fork new process");
        exit(EXIT_FAILURE);
    }
}

Process::~Process()
{
    //Exiting process on process object destruction
    exit(0);
}
