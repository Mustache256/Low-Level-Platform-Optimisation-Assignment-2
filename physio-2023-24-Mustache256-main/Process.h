#pragma once
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "Constants.h"

class Process
{
public:
    Process(int pipeI);
    Process(int pipeI, int boxIndexNum);
    ~Process();

    //Stores ID of this process
    pid_t processId;
    //Stores index associated with this process' pipe 
    int pipeIndex;

    //Box-Specific values
    int numOfBoxes;
    int boxIndex;

    bool tasksComplete;
};

