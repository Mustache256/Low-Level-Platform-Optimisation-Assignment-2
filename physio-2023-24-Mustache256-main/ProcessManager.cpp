#include "ProcessManager.h"

ProcessManager::ProcessManager(int numOfNewProcesses)
{
    for(int i = 0; i < numOfNewProcesses; i++)
    {
        CreateProcess(false);
    }
}

ProcessManager::ProcessManager(int numOfNewProcesses, bool initPipes)
{
    for(int i = 0; i < numOfNewProcesses; i++)
    {
        CreateProcess(initPipes);
    }
}

ProcessManager::~ProcessManager()
{
    //Destructing all processes
    for(Process* process : processes)
    {
        process->~Process();
    }
}

void ProcessManager::CreateProcess()
{
    Process* newProcess = new Process(false);
    processes.push_back(newProcess);
}

void ProcessManager::CreateProcess(bool InitPipe)
{
    Process* newProcess = new Process(InitPipe);
    processes.push_back(newProcess);
}

void ProcessManager::InsertProcess(Process* p)
{
    processes.push_back(p);
}

void ProcessManager::ExitProcess(int i)
{
    processes[i]->~Process();
}

void ProcessManager::ExitAllProcesses()
{
    for(Process* p : processes)
    {
        p->~Process();
    }
}

void ProcessManager::InitProcessPipe(int i)
{
    processes[i]->InitPipe();
}

void ProcessManager::InitAllProcessPipes()
{
    for(Process* p : processes)
    {
        p->InitPipe();
    }
}

Process* ProcessManager::GetProcess(int i)
{
    return processes[i];
}

vector<Process*> ProcessManager::GetProcesses()
{
    return processes;
}

int ProcessManager::GetProcessPipeRead(int i)
{
    return processes[i]->GetPipeRead();
}

int ProcessManager::GetProcessPipeWrite(int i)
{
    return processes[i]->GetPipeWrite();
}