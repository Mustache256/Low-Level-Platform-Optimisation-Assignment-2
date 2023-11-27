
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

int Process::GetPipeRead()
{
    return pipefd[0];
}

int Process::GetPipeWrite()
{
    return pipefd[1];
}