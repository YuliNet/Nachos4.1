/*
 * @Author: Lollipop
 * @Date: 2019-11-12 15:32:49
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-13 21:14:40
 * @Description: 
 */
#include "ThreadManager.h"
#include "main.h"

bool threadIDComp(void *target, void *data)
{
    int threadId = ((Thread *)target)->getPid();

    return (threadId == (int)data);
}

static void printThread(Thread* thread)
{
    const char* status[5] = {"Created", "Running", "Ready", "Blocked", "Zombie"};

	printf(" %8d | %10s | %6d | %s \n",
			thread->getPid(),
			thread->getName(),
			thread->getUid(),
			status[thread->getStatus()]);
}

ThreadManager::ThreadManager()
{
    threadCnt = 0;
    threadList = new List<Thread*>();
    threadMap = new Bitmap(THREAD_COUNT_MAX);
}

ThreadManager::~ThreadManager()
{
    delete threadMap;
    delete threadList;
}

Thread*
ThreadManager::createThread(char* threadName, int uid)
{
    Thread* newThread = NULL;

    if (threadCnt + 1 < THREAD_COUNT_MAX)
    {
        threadCnt++;

        int pid = generateThreadID();
        newThread = new Thread(threadName, uid, pid);

        if (threadCnt > 1)
        {
            kernel->currentThread->addChild(newThread);
        }

        threadList->Append(newThread);
    }

    return newThread; 
}

Thread*
ThreadManager::createThread(char* threadName)
{
    return createThread(threadName, 0);
}

void
ThreadManager::deleteThread(Thread* thread)
{
    if (thread != NULL)
    {
        threadCnt--;
        int pid = thread->getPid();
        threadMap->Clear(pid);
        threadList->Remove(thread);

        delete thread;
    }

}

int
ThreadManager::generateThreadID()
{
    return threadMap->FindAndSet();
}

Thread*
ThreadManager::getThreadPtr(int pid)
{
    if (!threadMap->Test(pid))
    {
        return NULL;
    }

    return (Thread*)(threadList->findByComp(threadIDComp, (void*)pid));
}

void
ThreadManager::listThreadStatus()
{
    printf("Command TS -> List Thread Status:\n");
	printf(" ThreadID | ThreadName | UserID | Status \n"); 
	printf(" -------- | ---------- | ------ | ------ \n");

    threadList->Apply(printThread);
}