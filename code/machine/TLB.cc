#include "translate.h"

TLB::TLB()
{
    for (int i = 0; i < 4; i++)
    {
        tlbPtr[i] = new TLBEntry[4];
    }
}

TLB::~TLB()
{
    for (int i = 0; i < 4; i++)
        delete[] tlbPtr[i];
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

    for (int i = 0; i < 4; i++)
    {
        if (!tlbPtr[TLBI][i].valid)
        {
            tlbPtr[TLBI][i].PPN = pageFrame;
            tlbPtr[TLBI][i].Tag = TLBT;
            tlbPtr[TLBI][i].valid = true;
            return;
        }
    }

    //TODO:更新TLB
}