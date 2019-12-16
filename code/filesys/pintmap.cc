// pbitmap.c 
//	Routines to manage a persistent bitmap -- a bitmap that is
//	stored on disk.
//
// Copyright (c) 1992,1993,1995 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "pintmap.h"
#include "utility.h"
#include "disk.h"
#include "main.h"
#include "synchdisk.h"
#include "debug.h"

#define FreeMapHeaderSector 	0
#define DirectoryHeaderSector 	1
#define FreeMapFileFirstSector  2
//----------------------------------------------------------------------
// PersistentIntmap::PersistentIntmap(int)
// 	Initialize a bitmap with "numItems" bits, so that every bit is clear.
//	it can be added somewhere on a list.
//
//	"numItems" is the number of bits in the bitmap.
//
//      This constructor does not initialize the bitmap from a disk file
//----------------------------------------------------------------------

PersistentIntmap::PersistentIntmap():IntMap(NumSectors)
{
}

void
PersistentIntmap::Init()
{
    Mark(FreeMapHeaderSector, -1);
    Mark(DirectoryHeaderSector, -1);

    int n = divRoundUp(NumSectors * sizeof(int), SectorSize);
    int t = FreeMapFileFirstSector;
    for (int i = 0; i < n; i++)
    {
        Mark(t, t+1);
        t++;
    }
    Mark(t - 1, -1);
    WriteBack();
}

//----------------------------------------------------------------------
// PersistentIntmap::~PersistentIntmap
// 	De-allocate a persistent bitmap.
//----------------------------------------------------------------------

PersistentIntmap::~PersistentIntmap()
{ 
}

//----------------------------------------------------------------------
// PersistentIntmap::FetchFrom
// 	Initialize the contents of a persistent bitmap from a Nachos file.
//----------------------------------------------------------------------
void
PersistentIntmap::FetchFrom() 
{
    int numSectors;
    char *buf;

    numSectors = divRoundUp(NumSectors * sizeof(int), SectorSize);
    buf = new char[numSectors * SectorSize];

    int sector = FreeMapFileFirstSector;
    for (int i = 0; i < numSectors; i++)
    {
        kernel->synchDisk->ReadSector(sector, &buf[i * SectorSize]);
        sector++;
    }
    
    bcopy(buf, map, numSectors * SectorSize);
    delete buf;
}

//----------------------------------------------------------------------
// PersistentIntmap::WriteBack
// 	Store the contents of a persistent bitmap to a Nachos file.
//普通文件通过打开文件的writeAt实现写回磁盘，在WtiteAt中需要根据intMap得到写入的物理磁盘号
//但是intMap写回磁盘有个自举问题，自己还没写回磁盘，但是writeAt又要从磁盘中读intMap
//所以intMap的写回操作需要特殊处理
//----------------------------------------------------------------------
void
PersistentIntmap::WriteBack()
{
    int numSectors;
    char *buf;

    numSectors = divRoundUp(NumSectors * sizeof(int), SectorSize);
    buf = new char[numSectors * SectorSize];
    
    bcopy(map, buf, numSectors * SectorSize);

    int sector = FreeMapFileFirstSector;
    for (int i = 0; i < numSectors; i++)
    {
        kernel->synchDisk->WriteSector(sector, &buf[i * SectorSize]);
        sector++;
    }
    delete buf;
}
