/*
 * @Author: Lollipop
 * @Date: 2019-11-12 10:44:37
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-15 12:07:45
 * @Description: 这里有一个问题：所有进程的虚拟页总和不能大于MAX_VIRT_PAGES，而且不同进程的虚拟地址大小不同
 */
#include "VirtMemManager.h"
#include "main.h"

VirtMemManager::VirtMemManager(int size)
{
    ASSERT(size > 0);

    virtPageNums = 0;
    virtMemTableSize = size;

    virtMemTable = new AddrSpace*[size];
    for (int i = 0; i < size; i++)
    {
        virtMemTable[i] = NULL;
    }
}

VirtMemManager::~VirtMemManager()
{
    delete[] virtMemTable;
}

AddrSpace*
VirtMemManager::createAddrSpace(int mainThreadId, char* filename)
{
    AddrSpace* entry;

    entry = virtMemTable[mainThreadId];
    if (entry != NULL)
    {
        return entry;
    }

    entry = new AddrSpace(mainThreadId, filename);

    int size = entry->getNumPages();
    if (virtPageNums + size > MAX_VIRT_PAGES) //这里所有的进程共用一个虚拟地址空间？为什么要限制全局虚拟页的数量？而且各个进程的虚拟地址空间大小不相同
    {
        delete entry;
        entry = NULL;
    }
    else
    {
        virtMemTable[mainThreadId] = entry;
        virtPageNums += size;
    }

    return entry;
}

/**
 * @description: 遍历进程的页表，将该进程占用的物理页清空, 然后删除进程的地址空间
 * @param {int threadId} 
 * @return: 
 */
void
VirtMemManager::deleteAddrSpace(int threadId)
{
    if (threadId >= 0 && threadId < virtMemTableSize)
    {
        AddrSpace* entry = virtMemTable[threadId];
        if (entry != NULL)
        {
            TranslationEntry* pageTable = entry->getPageTable();
            int size = entry->getNumPages();
            PhyMemManager* PhyManager = kernel->memoryManager->getPhyMemManager();
            for (int i = 0; i < size; i++)
            {
                if (pageTable[i].valid)
                {
                    PhyManager->clearOnePage(pageTable[i].physicalPage);

                }
            }

            delete entry;
            virtMemTable[threadId] = NULL;
        }
    }
}

AddrSpace*
VirtMemManager::getAddrSpaceOfThread(int threadId)
{
    if (threadId >= 0 && threadId < virtMemTableSize)
    {
        return virtMemTable[threadId];
    }
    else
    {
        return NULL;
    }
    
}