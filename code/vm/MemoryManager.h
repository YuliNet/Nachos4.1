/*
 * @Author: Lollipop
 * @Date: 2019-11-12 11:22:38
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-15 12:01:13
 * @Description: 
 */
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "VirtMemManager.h"
#include "PhyMemManager.h"

class MemoryManager
{
    public:
        MemoryManager();
        ~MemoryManager();

        void pageFaultHandler(int vpn);

        AddrSpace* getAddrSpaceOfThread(int threadId);
        AddrSpace* createAddrSpace(int threadId, char* filename);
        //TODO:shareSpace()
        void deleteAddrSpace(int threadId);

        VirtMemManager* getVirtMemManger() {return virtMemManager;}
        PhyMemManager* getPhyMemManager() {return phyMemManager;}

    private:
        VirtMemManager* virtMemManager;
        PhyMemManager* phyMemManager;
};

#endif// MEMORYMANAGER_H