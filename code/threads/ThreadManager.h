/*
 * @Author: Lollipop
 * @Date: 2019-11-12 15:28:45
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-13 00:19:46
 * @Description: 
 */
#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include "list.h"
#include "thread.h"
#include "bitmap.h"

#define THREAD_COUNT_MAX 128

class ThreadManager
{
    public:
        ThreadManager();
        ~ThreadManager();

        Thread* createThread(char* threadName);
        Thread* createThread(char* threadName, int uid);
        void deleteThread(Thread* thread);

        void listThreadStatus();
        int getThreadCnt() {return (threadCnt);}

        int generateThreadID();

        Thread* getThreadPtr(int pid);

    private:
        List<Thread*>* threadList;
        int threadCnt;
        Bitmap* threadMap;
};

#endif	// THREADMANAGER_H