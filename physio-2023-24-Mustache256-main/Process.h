#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "Constants.h"

class Process
{
public:
    Process(bool initPipe);
    Process(bool initPipe, int indexNum);
    ~Process();

    void InitPipe();

    pid_t processId;
    int pipefd[2];

    int numOfBoxes;
    int boxIndex;

    bool tasksComplete;
};

