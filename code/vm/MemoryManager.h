/*
 * @Author: Lollipop
 * @Date: 2019-11-12 11:22:38
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-12 14:39:54
 * @Description: 
 */
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "VirtMemManager.h"
#include "PhyMemManager.h"
#include "SwappingManager.h"

class MemoryManager
{
    public:
        MemoryManager();
        ~MemoryManager();

        //TODO:process()

        AddrSpace* getAddrSpaceOfThread(int threadId);
        AddrSpace* createAddrSpace(int threadId, OpenFile* executable);
        //TODO:shareSpace()
        void deleteAddrSpace(int threadId);

        VirtMemManager* getVirtMemManger() {return virtMemManager;}
        PhyMemManager* getPhyMemManager() {return phyMemManager;}

    private:
        VirtMemManager* virtMemManager;
        PhyMemManager* phyMemManager;
};

#endif// MEMORYMANAGER_H