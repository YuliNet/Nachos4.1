/*
 * @Author: Lollipop
 * @Date: 2019-11-11 21:50:43
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-11 21:52:16
 * @Description: LRU替换算法。维护一个上次使用时间数组，记录每一项上次使用的时间（等于系统时间totalticks）
 */
#ifndef SWAPPINGLRU_H
#define SWAPPINGLRU_H

#include "SwappingStrategy.h"

class SwappingLRU : public SwappingStrategy
{
private:
    int* lastUsedTimeTable;
    int tableSize;
public:
    SwappingLRU(int size);
    ~SwappingLRU();

    virtual int findOneElementToSwap();
    virtual void updateElementWeight(int index);
};

#endif	// SWAPPINGLRU_H