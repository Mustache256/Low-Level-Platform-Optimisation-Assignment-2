#pragma once
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

class Pipe
{
private:
    int pipefd[2];
public:
    Pipe();
    ~Pipe();

    int GetReadEnd();
    int GetWriteEnd();

    void WriteFloat(float f);
};