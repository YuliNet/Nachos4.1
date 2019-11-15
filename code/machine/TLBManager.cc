/*
 * @Author: Lollipop
 * @Date: 2019-11-15 14:02:48
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-15 14:14:20
 * @Description: 
 */
#include "translate.h"
#include "debug.h"
#include "TLBManager.h"
#include "main.h"

TLBManager::TLBManager()
{
    //4路相连，16个Entry
    for (int i = 0; i < 4; i++)
    {
        tlbPtr[i] = new TLBEntry[4];
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tlbPtr[i][j].valid = false;
            tlbPtr[i][j].threadId = -1;
        }
    }
}

TLBManager::~TLBManager()
{
    for (int i = 0; i < 4; i++)
    {
        delete[] tlbPtr[i];
    }
}

int TLBManager::translate(int virtAddr)
{
    unsigned int TLBT, TLBI;
    unsigned int vpn, offset;
    int physAddr = -1;

    vpn = (unsigned)virtAddr / PageSize;
    offset = (unsigned)virtAddr % PageSize;

    TLBI = vpn & 0x3;
    TLBT = (vpn >> 2) & 0x3FFFFFFF;

    for (int i = 0; i < 4; i++)
    {
        if (tlbPtr[TLBI][i].valid && (tlbPtr[TLBI][i].Tag == TLBT))
        {
            tlbPtr[TLBI][i].lru = 0;
            physAddr = tlbPtr[TLBI][i].PPN * PageSize + offset;
            break;
        }
    }
    return physAddr;
}

void TLBManager::update(int virtAddr, int pageFrame)
{
    unsigned int vpn;
    unsigned int TLBT, TLBI;

    vpn = (unsigned)virtAddr / PageSize;
    TLBI = vpn & 0x3;
    TLBT = (vpn >> 2) & 0x3FFFFFFF;

    //替换的下标
    int index = 0;
    for (int i = 0; i < 4; i++)
    {
        if (tlbPtr[TLBI][i].valid)
        {
            tlbPtr[TLBI][i].lru++;
            if (tlbPtr[TLBI][i].lru > tlbPtr[TLBI][index].lru)
                index = i;
        }
        else
        {
            index = i;
            break;
        }
    }
    if (tlbPtr[TLBI][index].valid)
    {
        DEBUG(dbgLru, "replace tlb ");
    }
    else
    {
        DEBUG(dbgLru, "update tlb ");
    }
    
    tlbPtr[TLBI][index].PPN = pageFrame;
    tlbPtr[TLBI][index].Tag = TLBT;
    tlbPtr[TLBI][index].valid = true;
    tlbPtr[TLBI][index].lru = 0;
    tlbPtr[TLBI][index].threadId = kernel->currentThread->getPid();
}

void TLBManager::invalidEntry(int threadId, int vpn)
{
    unsigned int TLBT, TLBI;
    TLBI = vpn & 0x3;
    TLBT = (vpn >> 2) & 0x3FFFFFFF;

    for (int i = 0; i < 4; i++)
    {
        if (tlbPtr[TLBI][i].valid && tlbPtr[TLBI][i].Tag == TLBT && tlbPtr[TLBI][i].threadId == threadId)
        {
            tlbPtr[TLBI][i].valid = FALSE;
        }
    }
}