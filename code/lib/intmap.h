#ifndef INTMAP_H
#define INTMAP_H

class IntMap
{
public:
    IntMap(int numItems);
    ~IntMap();

    void Mark(int which);
    void Mark(int which, int num);
    void Clear(int which);
    bool Test(int which) const;
    int FindAndSet();
    int FindAndSet(int num);
    int NumClear() const;
    int Get(int which) const;

    void Print() const;

protected:
    int numItems;
    int* map;
};

#endif	// INTMAP_H