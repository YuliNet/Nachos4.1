/*
 * @Author: Lollipop
 * @Date: 2019-11-17 09:45:43
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-18 11:49:37
 * @Description: 
 */

#include "synch.h"
#include "main.h"
#include "thread.h"
#include "debug.h"
#include "stdlib.h"

class Buffer;
#define N 3
Buffer *buffer;

class Buffer
{
public:
    Buffer(int size);
    ~Buffer();

    void insert(int item);
    int remove();
    int Count();

private:
    int size;
    int *data;
    int index;
};

Buffer::Buffer(int size)
{
    this->size = size;
    data = new int[size];
    index = 0;
}

Buffer::~Buffer()
{
    delete data;
}

int Buffer::Count()
{
    return index;
}

void Buffer::insert(int item)
{
    data[index++] = item;
    DEBUG(dbgSynch, "producer insert " << item << " at " << index - 1);
}

int Buffer::remove()
{
    DEBUG(dbgSynch, "consumer remove " << data[index - 1]);
    return data[--index];
}

int produce_item()
{
    return rand();
}

Semaphore *Sema_mutex;
Semaphore *Sema_empty;
Semaphore *Sema_full;

void Semaphore_produce(int *n)
{
    int item;

    for (int i = 0; i < *n; i++)
    {
        item = produce_item();
        Sema_empty->P();
        Sema_mutex->P();
        buffer->insert(item);
        Sema_mutex->V();
        Sema_full->V();
    }
}

void Semaphore_consume(int *n)
{
    int item;

    for (int i = 0; i < *n; i++)
    {
        Sema_full->P();
        Sema_mutex->P();
        item = buffer->remove();
        Sema_mutex->V();
        Sema_empty->V();
    }
}

void SemaphoreProduceComsumeTest()
{
    buffer = new Buffer(N);
    Sema_full = new Semaphore("full", 0);
    Sema_empty = new Semaphore("empty", N);
    Sema_mutex = new Semaphore("mutex", 1);

    Thread *producer = kernel->threadManager->createThread("producer");
    Thread *consumer = kernel->threadManager->createThread("consumer");

    int n = 10;
    producer->Fork((VoidFunctionPtr)Semaphore_produce, &n);
    consumer->Fork((VoidFunctionPtr)Semaphore_consume, &n);

    while (1)
    {
        kernel->currentThread->Yield();
    }
}

Condition *Cond_NotEmpty;
Condition *Cond_NotFull;
Lock *lock;

void Condition_produce(int *n)
{
    int item;

    for (int i = 0; i < *n; i++)
    {
        item = produce_item();
        lock->Acquire();
        if (buffer->Count() == N)
            Cond_NotFull->Wait(lock);
        buffer->insert(item);
        if (buffer->Count() == 1)
            Cond_NotEmpty->Signal(lock);
        lock->Release();
    }
}

void Condition_consume(int *n)
{
    int item;

    for (int i = 0; i < *n; i++)
    {
        lock->Acquire();
        if (buffer->Count() == 0)
            Cond_NotEmpty->Wait(lock);
        item = buffer->remove();
        if (buffer->Count() == N - 1)
            Cond_NotFull->Signal(lock);
        lock->Release();
    }
}

void ConditionProduceConsumerTest()
{
    buffer = new Buffer(N);
    lock = new Lock("mutex lock");
    Cond_NotEmpty = new Condition("buffer not empty");
    Cond_NotFull = new Condition("buffer not full");

    Thread *producer = kernel->threadManager->createThread("producer");
    Thread *consumer = kernel->threadManager->createThread("consumer");

    int n = 10;

    producer->Fork((VoidFunctionPtr)Condition_produce, &n);

    consumer->Fork((VoidFunctionPtr)Condition_consume, &n);

    while (1)
    {
        kernel->currentThread->Yield();
    }
}



int num;       //共享数据区
int readerCnt; //记录当前的读者数量
Semaphore *numMutex;
Semaphore *cntMutex;

void Semaphore_read(void *)
{
    for(int i = 0; i < 10; i++)
    {
        cntMutex->P();
        readerCnt++;
        if (readerCnt == 1)
        {
            numMutex->P();
        }
        cntMutex->V();

        //模拟读者read过程
        DEBUG(dbgSynch, "read num : " << num);

        cntMutex->P();
        readerCnt--;
        if (readerCnt == 0)
        {
            numMutex->V();
        }
        cntMutex->V();
        kernel->currentThread->Yield();
    }
}

void Semaphore_write(void *)
{
    for(int i = 0; i < 10; i++)
    {
        numMutex->P();
        num = produce_item();
        DEBUG(dbgSynch, "write : " << num);
        numMutex->V();
        kernel->currentThread->Yield();
    }
}

void SemaphoreRWTest()
{
    readerCnt = 0;
    numMutex = new Semaphore("numMutex", 1);
    cntMutex = new Semaphore("cntMutex", 1);

    Thread *reader1 = kernel->threadManager->createThread("reader1");
    Thread *reader2 = kernel->threadManager->createThread("reader2");
    Thread *reader3 = kernel->threadManager->createThread("reader3");
    Thread *writer = kernel->threadManager->createThread("writer");

    reader1->Fork((VoidFunctionPtr)Semaphore_read, &readerCnt);
    reader2->Fork((VoidFunctionPtr)Semaphore_read, &readerCnt);
    reader3->Fork((VoidFunctionPtr)Semaphore_read, &readerCnt);
    writer->Fork((VoidFunctionPtr)Semaphore_write, &readerCnt);

    while (1)
    {
        kernel->currentThread->Yield();
    }
}

Barrier* barrier;

void simpleThread(int* n)
{
        DEBUG(dbgSynch, "enter simpleThread");
        barrier->Wait();
        DEBUG(dbgSynch, "after barrier");
}

void BarrierTest()
{
    barrier = new Barrier(3);

    Thread* thread1 = kernel->threadManager->createThread("thread1");
    Thread* thread2 = kernel->threadManager->createThread("thread2");
    Thread* thread3 = kernel->threadManager->createThread("thread3");
    
    int n = 1;
    thread1->Fork((VoidFunctionPtr)simpleThread, &n);
    thread2->Fork((VoidFunctionPtr)simpleThread, &n);
    thread3->Fork((VoidFunctionPtr)simpleThread, &n);

    while (1)
    {
        kernel->currentThread->Yield();
    }
}


rwLock* rwlock;
int RwTest_num;

void rwLockReader(int* n)
{
    for (int i = 0; i < *n; i++)
    {
        rwlock->read_lock();
        DEBUG(dbgSynch, "read : " << RwTest_num);
        rwlock->read_unlock();
        kernel->currentThread->Yield();
    }
}

void rwLockWrite(int* n)
{
    for (int i = 0; i < *n; i++)
    {
        rwlock->write_lock();
        RwTest_num = produce_item();
        DEBUG(dbgSynch, "write : " << RwTest_num);
        rwlock->write_unlock();
        kernel->currentThread->Yield();
    }
}

void rwLockTest()
{
    rwlock = new rwLock();

    Thread* reader1 = kernel->threadManager->createThread("reader1");
    Thread* reader2 = kernel->threadManager->createThread("reader2");
    Thread* reader3 = kernel->threadManager->createThread("reader3");
    
    Thread* writer1 = kernel->threadManager->createThread("writer1");
    Thread* writer2 = kernel->threadManager->createThread("writer2");
    
    int n = 3;
    reader1->Fork((VoidFunctionPtr)rwLockReader, &n);
    reader2->Fork((VoidFunctionPtr)rwLockReader, &n);
    writer1->Fork((VoidFunctionPtr)rwLockWrite, &n);
    writer2->Fork((VoidFunctionPtr)rwLockWrite, &n);
    reader3->Fork((VoidFunctionPtr)rwLockReader, &n);
    
    while(1)
    {
        kernel->currentThread->Yield();
    }
}



