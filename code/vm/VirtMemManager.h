/*
 * @Author: Lollipop
 * @Date: 2019-11-12 10:31:32
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-12 11:28:00
 * @Description: 全局虚存管理器。维护一个virtMemTable数组记录每个进程的AddrSpace指针，进程ID作为数组下标
 */
#ifndef VIRTMEMMANAGER_H
#define VIRTMEMMANAGER_H 

#include "addrspace.h"

#define MAX_VIRT_PAGES 4096

class VirtMemManager
{
private:
    int virtPageNums;
    int virtMemTableSize;
    AddrSpace** virtMemTable;
public:
    VirtMemManager(int size);
    ~VirtMemManager();

    AddrSpace* createAddrSpace(int mainThreadId, OpenFile* executable);
    AddrSpace* getAddrSpaceOfThread(int threadId);
    //TODO:shareAddrSpace()
    void deleteAddrSpace(int threadId);
};

#endif	// VIRTMEMMANGER_H
