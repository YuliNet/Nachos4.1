/*
 * @Author: Lollipop
 * @Date: 2019-11-11 21:20:20
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-11 21:21:19
 * @Description: 替换策略，可以用在TLB、页表等有替换算法的地方
 */
#ifndef SWAPPINGSTRATEGY_H
#define SWAPPINGSTRATEGY_H

class SwappingStrategy
{
    public:
        virtual int findOneElementToSwap() = 0;
        virtual void updateElementWeight(int index) = 0;
};

#endif	// SWAPPINGSTRATEGY_H