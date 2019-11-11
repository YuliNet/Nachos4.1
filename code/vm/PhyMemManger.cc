/*
 * @Author: Lollipop
 * @Date: 2019-11-11 22:12:35
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-11 22:35:08
 * @Description: 
 */
#include "PhyMemManger.h"
#include "SwappingLRU.h"

PhyMemManger::PhyMemManger(int pageNums)
{
    phyPageNums = pageNums;
    phyMemoryMap = new Bitmap(pageNums);
    phyMemPageTable = new PhyMemPageEntry[pageNums];
    swappingStrategy = new SwappingLRU(pageNums);
}

PhyMemManger::~PhyMemManger()
{
    delete phyMemoryMap;
    delete [] phyMemPageTable;
    delete swappingStrategy;
}

int
PhyMemManger::findOneEmptyPage()
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
PhyMemManger::swapOnePage()
{
    return swappingStrategy->findOneElementToSwap();
}

void
PhyMemManger::clearOnePage(int phyPage)
{
    phyMemoryMap->Clear(phyPage);
}

bool
PhyMemManger::isPageValid(int phyPage)
{
    return phyMemoryMap->Test(phyPage);
}

int 
PhyMemManger::getMainThread(int phyPage)
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
PhyMemManger::setMainThreadId(int phyPage, int threadId)
{
    if (phyMemoryMap->Test(phyPage))
    {
        phyMemPageTable[phyPage].mainThreadId = threadId;
    }
}

int
PhyMemManger::getVirtualPage(int phyPage)
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
PhyMemManger::setVirtualPage(int phyPage, int virtualPage)
{
    if (phyMemoryMap->Test(phyPage))
    {
        phyMemPageTable[phyPage].virtualPage = virtualPage;
    }
}

void
PhyMemManger::updatePageWeight(int phyPage)
{
    if (phyMemoryMap->Test(phyPage))
    {
        swappingStrategy->updateElementWeight(phyPage);
    }
}
