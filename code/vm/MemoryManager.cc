/*
 * @Author: Lollipop
 * @Date: 2019-11-12 14:42:14
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-15 14:17:36
 * @Description: 
 */
#include "main.h"
#include "MemoryManager.h"
#include "ThreadManager.h"

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
MemoryManager::createAddrSpace(int threadId, char* filename)
{
    return virtMemManager->createAddrSpace(threadId, filename);
}

void
MemoryManager::deleteAddrSpace(int threadId)
{
    virtMemManager->deleteAddrSpace(threadId);
}

void
MemoryManager::pageFaultHandler(int vpn)
{
    int currentThreadId = kernel->currentThread->getPid();
    AddrSpace* currentThreadAddrSpace = virtMemManager->getAddrSpaceOfThread(currentThreadId);
    TranslationEntry* currentPageTable = currentThreadAddrSpace->getPageTable();

    if (!currentPageTable[vpn].valid)
    {
        int swapPhyPage = phyMemManager->findOneEmptyPage();

        // swapPhyPage == -1, 表示当前物理页框全都被占用，需要使用替换算法找到一个进行替换
        if (swapPhyPage == -1)
        {
            swapPhyPage = phyMemManager->swapOnePage();
            int swapThreadId = phyMemManager->getMainThread(swapPhyPage);
            int swapVirtPage = phyMemManager->getVirtualPage(swapPhyPage);
            TranslationEntry* swapPageTable = virtMemManager->getAddrSpaceOfThread(swapThreadId)->getPageTable();

            //脏页需要写回磁盘
            if (swapPageTable[swapVirtPage].dirty)
            {
                AddrSpace* swapThreadAddrSpace = virtMemManager->getAddrSpaceOfThread(swapThreadId);
                OpenFile* swapFile = swapThreadAddrSpace->getExeFileId();
                swapFile->WriteAt(&(kernel->machine->mainMemory[swapPhyPage * PageSize]),
                            PageSize,
                            swapVirtPage * PageSize + sizeof(NoffHeader));
                #ifdef USE_TLB
                kernel->machine->tlbManager->invalidEntry(swapVirtPage, swapThreadId);
                #endif
            }

            swapPageTable[swapVirtPage].valid = FALSE;
        }

        phyMemManager->setVirtualPage(swapPhyPage, vpn);
        phyMemManager->setMainThreadId(swapPhyPage, currentThreadId);
        phyMemManager->updatePageWeight(swapPhyPage);

        currentPageTable[vpn].valid = TRUE;
        currentPageTable[vpn].physicalPage = swapPhyPage;

        //从磁盘上把该页读入内存
        OpenFile* executable = kernel->currentThread->space->getExeFileId();
        executable->ReadAt(&(kernel->machine->mainMemory[swapPhyPage * PageSize]),
						    PageSize,
							vpn * PageSize + sizeof(NoffHeader));
    }
    
}