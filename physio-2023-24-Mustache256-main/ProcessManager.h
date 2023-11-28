#include <iostream>
#include <vector>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "Process.h"

using namespace std;

class ProcessManager
{
private:
    vector<Process*> processes;

    int boxesPerProcess;
    int index = 0;
public:
    ProcessManager();
    ProcessManager(pid_t mainProcessId, int numOfNewProcesses);
    ProcessManager(pid_t mainProcessId, int numOfNewProcesses, bool initPipes);
    ~ProcessManager();

    void CreateProcess();
    void CreateProcess(bool initPipe);
    void InsertProcess(Process* p);
    void ExitProcess(int i);
    void ExitAllProcesses();
    void InitProcessPipe(int i);
    void InitAllProcessPipes();

    Process* GetProcess(int i);
    Process* GetProcessById(pid_t id);
    vector<Process*> GetProcesses();
    //int GetProcessPipeRead(int i);
    //int GetProcessPipeWrite(int i);

    bool CheckTasksCompleted();
};


