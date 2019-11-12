/*
 * @Author: Lollipop
 * @Date: 2019-11-12 14:42:14
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-12 14:47:08
 * @Description: 
 */
#include "MemoryManager.h"

MemoryManager::MemoryManager()
{
    virtMemManager = new VirtMemManager(THREAD_COUNT_MAX);
    phyMemManager  = new PhyMemManager(NumPhysPages);
}

MemoryManager::~MemoryManager()
{
    delete virtMemManager;
    delete phyMemManager;
}

AddrSpace*
MemoryManager::getAddrSpaceOfThread(int threadId)
{
    return (virtMemManager)->getAddrSpaceOfThread(threadId);
}

AddrSpace*
MemoryManager::createAddrSpace(int threadId, OpenFile* executable)
{
    return virtMemManager->createAddrSpace(threadId, executable);
}

AddrSpace*
MemoryManager::deleteAddrSpace(int threadId)
{
    virtMemManager->deleteAddrSpace(threadId);
}