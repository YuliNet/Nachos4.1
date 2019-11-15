/*
 * @Author: Lollipop
 * @Date: 2019-11-15 13:56:04
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-15 14:08:48
 * @Description: 
 */
#ifndef TLBMANAGER_H
#define TLBMANAGER_H

class TLBEntry
{
public:
    unsigned int Tag;
    int PPN;
    bool valid;
    int threadId;

    unsigned int lru;

    TLBEntry()
    {
        valid = false;
    }
};

class TLBManager
{
public:
    TLBEntry *tlbPtr[4];

    TLBManager();
    ~TLBManager();
    int translate(int virtAddr);
    void update(int virtAddr, int pageFrame);
    void invalidEntry(int threadId, int vpn);
};
#endif // TLBMANAGEH