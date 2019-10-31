#include "translate.h"
#include "debug.h"
TLB::TLB()
{
    //4路相连，16个Entry
    for (int i = 0; i < 4; i++)
    {
        tlbPtr[i] = new TLBEntry[4];
    }
}

TLB::~TLB()
{
    for (int i = 0; i < 4; i++)
    {
        delete[] tlbPtr[i];
    }
}

int TLB::translate(int virtAddr)
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

void TLB::update(int virtAddr, int pageFrame)
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
}