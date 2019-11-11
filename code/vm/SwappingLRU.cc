/*
 * @Author: Lollipop
 * @Date: 2019-11-11 21:56:08
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-11 22:28:21
 * @Description: 
 */

#include "SwappingLRU.h"
#include "main.h"

SwappingLRU::SwappingLRU(int size)
{
    tableSize = size;
    lastUsedTimeTable = new int[size];

    for (int i = 0;  i < size;  i++)
    {
        lastUsedTimeTable[i] = -1;
    }
    
}

SwappingLRU::~SwappingLRU()
{
    delete[] lastUsedTimeTable;
}

int
SwappingLRU::findOneElementToSwap()
{
    int min;
    int target;

    for (int i = 0; i < tableSize; i++)
    {
        if (i == 0 || lastUsedTimeTable[i] < min)
        {
            min = lastUsedTimeTable[i];
            target = i;
        }    
    }

    return target;
}

void
SwappingLRU::updateElementWeight(int index)
{
    lastUsedTimeTable[index] = kernel->stats->totalTicks;
}