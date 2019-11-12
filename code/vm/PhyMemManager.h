/*
 * @Author: Lollipop
 * @Date: 2019-11-11 20:45:25
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-12 11:29:12
 * @Description: 用于管理物理内存的数据结构。使用位图记录物理页框的分配情况。
 *               使用PhyMemPageEntry记录物理页框所属的线程和对应的逻辑页号。(感觉这里相当于实现了倒排页表？)
 */

#ifndef PHYMEMMANAGER_H
#define PHYMEMMANAGER_H

#include "bitmap.h"
#include "SwappingStrategy.h"

class PhyMemPageEntry
{
    public:
        int mainThreadId;
        int virtualPage;
};

class PhyMemManager
{
    public:
        PhyMemManager(int pageNums);
        ~PhyMemManager();

        int findOneEmptyPage();
        int swapOnePage();
        void clearOnePage(int phyPage);
        bool isPageValid(int phyPage);

        int getMainThread(int phyPage);
        void setMainThreadId(int phyPage, int threadId);
        int getVirtualPage(int phyPage);
        void setVirtualPage(int phyPage, int virtualPage);
        void updatePageWeight(int phyPage);

    private:
        int phyPageNums;
        Bitmap* phyMemoryMap;
        PhyMemPageEntry* phyMemPageTable;
        SwappingStrategy* swappingStrategy;
};

#endif