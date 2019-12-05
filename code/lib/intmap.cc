#include "intmap.h"
#include "debug.h"

IntMap::IntMap(int numItems)
{
    this->numItems = numItems;
    map = new int[numItems];
    for (int i = 0; i < numItems; i++)
    {
        Clear(i);
    }
}

IntMap::~IntMap()
{
    delete map;
}

void
IntMap::Mark(int which)
{
    ASSERT(which >= 0 && which < numItems);
    map[which] = -1;
    ASSERT(Test(which));
}

void
IntMap::Mark(int which, int num)
{
    ASSERT(which >= 0 && which < numItems);
    map[which] = num;
    ASSERT(Test(which));
}

void
IntMap::Clear(int which)
{
    ASSERT(which >= 0 && which < numItems);
    map[which] = 0;
    ASSERT(!Test(which));
}

bool
IntMap::Test(int which) const
{
    ASSERT(which >= 0 && which < numItems);
    if (map[which] != 0)
    {
        return TRUE;
    }
    return FALSE;
}

int
IntMap::FindAndSet()
{
    for (int i = 0; i < numItems; i++)
    {
        if (!Test(i))
        {
            Mark(i);
            return i;
        }
    }
    return -1;
}

int
IntMap::FindAndSet(int num)
{
    for (int i = 0; i < numItems; i++)
    {
        if (!Test(i))
        {
            Mark(i, num);
            return i;
        }
    }
    return -1;
}

int
IntMap::NumClear() const
{
    int cnt = 0;
    for (int i = 0; i < numItems; i++)
    {
        if (!Test(i))
            cnt++;
    }
    return cnt;
}

void
IntMap::Print() const
{
    cout << "Intmap set:\n"; 
    for (int i = 0; i < numItems; i++) {
	if (Test(i)) {
	    cout << i << " : " << map[i] << endl;
	}
    }
    cout << "\n"; 
}

int
IntMap::Get(int which) const
{
    ASSERT(which < numItems);
    return map[which];
}