/*
 * @Author: Lollipop
 * @Date: 2019-11-11 22:12:35
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-12 11:29:52
 * @Description: 
 */
#include "PhyMemManager.h"
#include "SwappingLRU.h"

PhyMemManager::PhyMemManager(int pageNums)
{
    phyPageNums = pageNums;
    phyMemoryMap = new Bitmap(pageNums);
    phyMemPageTable = new PhyMemPageEntry[pageNums];
    swappingStrategy = new SwappingLRU(pageNums);
}

PhyMemManager::~PhyMemManager()
{
    delete phyMemoryMap;
    delete [] phyMemPageTable;
    delete swappingStrategy;
}

int
PhyMemManager::findOneEmptyPage()
{
    int page;

    page = phyMemoryMap->FindAndSet();

    return page;
}

/**
 * @description: 根据LRU替换算法得到一个被替换项的index。
 *               注意：这里仅仅得到下标，而没有完成替换。
 * @param none 
 * @return: 被替换项的index
 */
int
PhyMemManager::swapOnePage()
{
    return swappingStrategy->findOneElementToSwap();
}

void
PhyMemManager::clearOnePage(int phyPage)
{
    phyMemoryMap->Clear(phyPage);
}

bool
PhyMemManager::isPageValid(int phyPage)
{
    return phyMemoryMap->Test(phyPage);
}

int 
PhyMemManager::getMainThread(int phyPage)
{
    if (phyMemoryMap->Test(phyPage))
    {
        return phyMemPageTable[phyPage].mainThreadId;
    }
    else
    {
        return -1;
    }
}

void
PhyMemManager::setMainThreadId(int phyPage, int threadId)
{
    if (phyMemoryMap->Test(phyPage))
    {
        phyMemPageTable[phyPage].mainThreadId = threadId;
    }
}

int
PhyMemManager::getVirtualPage(int phyPage)
{
    if (phyMemoryMap->Test(phyPage))
    {
        return phyMemPageTable[phyPage].virtualPage;
    }
    else
    {
        return -1;
    }
    
    
}

void
PhyMemManager::setVirtualPage(int phyPage, int virtualPage)
{
    if (phyMemoryMap->Test(phyPage))
    {
        phyMemPageTable[phyPage].virtualPage = virtualPage;
    }
}

void
PhyMemManager::updatePageWeight(int phyPage)
{
    if (phyMemoryMap->Test(phyPage))
    {
        swappingStrategy->updateElementWeight(phyPage);
    }
}
