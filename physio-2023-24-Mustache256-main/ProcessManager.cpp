#include "ProcessManager.h"

ProcessManager::ProcessManager(pid_t mainProcessId, int numOfNewProcesses)
{
    boxesPerProcess = NUMBER_OF_BOXES / NUMBER_OF_PHYS_PROCESSES;

    for(int i = 0; i < numOfNewProcesses; i++)
    {
        if(getpid() == mainProcessId)
        {
            CreateProcess(false);
        } 
    }
}

ProcessManager::ProcessManager(pid_t mainProcessId, int numOfNewProcesses, bool initPipes)
{
    for(int i = 0; i < numOfNewProcesses; i++)
    {
        if(getpid() == mainProcessId)
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
    Process* newProcess = new Process(false, index);
    processes.push_back(newProcess);
    index += boxesPerProcess;
}

void ProcessManager::CreateProcess(bool InitPipe)
{
    Process* newProcess = new Process(InitPipe, index);
    processes.push_back(newProcess);
    index += boxesPerProcess;
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

Process* ProcessManager::GetProcessById(pid_t id)
{
    for(Process* process : processes)
    {
        if(process->processId == id)
            return process;
    }

    printf("\nTrying to access a process that is not within ProcessManager that is calling GetProcessById\n");
    return nullptr;
}

vector<Process*> ProcessManager::GetProcesses()
{
    return processes;
}

bool ProcessManager::CheckTasksCompleted()
{
    int count;

    for(int i = 0; i < processes.size(); i++)
    {
        if(processes[i]->tasksComplete)
            count++;
    }

    return count == processes.size();
}