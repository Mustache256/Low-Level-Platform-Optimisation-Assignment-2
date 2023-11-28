#pragma once
#include <iostream>
#include <vector>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "Process.h"
#include "Pipe.h"

using namespace std;

class ProcessManager
{
private:
    vector<Process*> processes;
    vector<Pipe*> pipes;

    int boxesPerProcess;
    int index = 0;
public:
    ProcessManager();
    ProcessManager(pid_t mainProcessId, int numOfNewProcesses, bool initPipes);
    ~ProcessManager();
    
    //void CreateProcess(bool initPipe);
    void CreateProcess(int pipeIndex);
    void InsertProcess(Process* p);
    void ExitProcess(int i);
    void ExitAllProcesses();
    //void InitProcessPipe(int i);
    //void InitAllProcessPipes();

    Process* GetProcess(int i);
    Process* GetProcessById(pid_t id);
    vector<Process*> GetProcesses();
    Pipe* GetPipe(int index);
    vector<Pipe*> GetPipes();
    int GetBoxesPerProcess();

    bool CheckTasksCompleted();
};


