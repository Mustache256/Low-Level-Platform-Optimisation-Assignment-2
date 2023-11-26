#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

class Process
{
private:
    pid_t processId;
    int pipefd[2];
public:
    Process(bool initPipe);
    ~Process();

    void InitPipe();
    int GetPipeWrite();
    int GetPipeRead();
};

