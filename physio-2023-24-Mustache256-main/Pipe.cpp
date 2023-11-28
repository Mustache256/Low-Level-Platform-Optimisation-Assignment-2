#include "Pipe.h"

Pipe::Pipe()
{
    if(pipe(pipefd) < 0)
    {
        printf("Unable to create pipe");
        exit(EXIT_FAILURE);
    }
}

Pipe::~Pipe()
{

}

int Pipe::GetReadEnd()
{
    return pipefd[0];
}

int Pipe::GetWriteEnd()
{
    return pipefd[1];
}

void Pipe::WriteFloat(float f)
{
    if(write(pipefd[1], &f, sizeof(float)) == -1)
    {
        printf("Error with writing to pipe");
    }
}