#include "Pipe.h"

Pipe::Pipe()
{
    //Creates pipe using file decriptor and checks that pipe was made successfully
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
    //Writes float to pipe and checks write was successful
    if(write(pipefd[1], &f, sizeof(float)) == -1)
    {
        printf("Error with writing to pipe");
    }
}