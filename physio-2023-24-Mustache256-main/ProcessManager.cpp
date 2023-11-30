#include "ProcessManager.h"

ProcessManager::ProcessManager()
{
    boxesPerProcess = NUMBER_OF_BOXES / NUMBER_OF_PHYS_PROCESSES;
}

ProcessManager::ProcessManager(pid_t mainProcessId, int numOfNewProcesses, bool initPipes)
{
    //Calculating number of boxes each process will need to handle
    boxesPerProcess = NUMBER_OF_BOXES / NUMBER_OF_PHYS_PROCESSES;

    //Creating a new pipe for each process that will be made
    for(int i = 0; i < numOfNewProcesses; i++)
    {
        Pipe* p = new Pipe();
        pipes.push_back(p);
    }

    //Creating the new child processes
    for(int i = 0; i < numOfNewProcesses; i++)
    {
        if(getpid() == mainProcessId)
            CreateProcess(i);
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

void ProcessManager::CreateProcess(int pipeIndex)
{
    //Create new process, push it back into the processes vector and increment index value for use in telling the next process where in the box vector to start from when updating box physics
    Process* newProcess = new Process(pipeIndex, index);
    processes.push_back(newProcess);
    index += boxesPerProcess;
}

void ProcessManager::InsertProcess(Process* p)
{
    //Add process to processes vector
    processes.push_back(p);
}

void ProcessManager::ExitProcess(int i)
{
    //Destroy process by index
    processes[i]->~Process();
}

void ProcessManager::ExitAllProcesses()
{
    //Destroy all processes in processes vector
    for(Process* p : processes)
    {
        p->~Process();
    }
}

Process* ProcessManager::GetProcess(int i)
{
    //Gt process by index
    return processes[i];
}

Process* ProcessManager::GetProcessById(pid_t id)
{
    //Get process by ID, search processes vector until process with matching ID is found and return it
    for(Process* process : processes)
    {
        if(process->processId == id)
            return process;
    }

    //If no process with matching ID is found, output error message
    printf("\nTrying to access a process that is not within ProcessManager that is calling GetProcessById\n");
    return nullptr;
}

vector<Process*> ProcessManager::GetProcesses()
{
    //Get processes vector
    return processes;
}

Pipe* ProcessManager::GetPipe(int index)
{
    //Get pipe by index
    return pipes[index];
}

vector<Pipe*> ProcessManager::GetPipes()
{
    //Get pipes vector
    return pipes;
}

int ProcessManager::GetBoxesPerProcess()
{
    //Get number of boxes each process handles
    return boxesPerProcess;
}

bool ProcessManager::CheckTasksCompleted()
{
    
    int count;

    //Loops through processes in processes vector and increment count if that process has completed its tasks, returns true if count is equal to size of process vector
    for(int i = 0; i < processes.size(); i++)
    {
        if(processes[i]->tasksComplete)
            count++;
    }

    return count == processes.size();
}