// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"
#include "pintmap.h"

FileHeader::FileHeader()
{
}

FileHeader::FileHeader(char* filename, char* filepath, FileType type, int sector)
{
    capacity = 0;
    numSectors = 0;
    limit = 0;
    firstSector = -1;
    lastSector = -1;
    this->type = type;
    selfSector = sector;
    strncpy(name, filename, FileNameMaxLen);
    strncpy(this->filepath, filepath, FilePathMaxLen);
}


//----------------------------------------------------------------------
// FileHeader::Allocate
// 	升级版，申请fileSize个字节，既可以在初始化文件时使用，也可以在文件容量不够时扩充文件容量
//----------------------------------------------------------------------

bool
FileHeader::Allocate(int fileSize)
{
    if (fileSize == 0)return TRUE;
    PersistentIntmap* freeMap = new PersistentIntmap();
    freeMap->FetchFrom();

    int n  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < n)
	    return FALSE;		// not enough space

    if (firstSector == -1)
    {
        firstSector = freeMap->FindAndSet();
        lastSector = firstSector;
        numSectors = 1;
        n--;
    }
    
    for (int i = 0; i < n; i++)
    {
        int next = freeMap->FindAndSet();
        freeMap->Mark(lastSector, next);
        lastSector = next;
    }

    numSectors += n; 
    capacity += fileSize;
    freeMap->WriteBack();
    delete freeMap;
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate()
{
    PersistentIntmap* freeMap = new PersistentIntmap();
    freeMap->FetchFrom();
    
    int now = firstSector;
    for (int i = 0; i < numSectors; i++)
    {
        ASSERT(freeMap->Test(now));
        int next = freeMap->Get(now);
        freeMap->Clear(now);
        now = next;
    }
    freeMap->WriteBack();
    delete freeMap;
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    kernel->synchDisk->ReadSector(sector, (char *)this);
    selfSector = sector;
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack()
{
    kernel->synchDisk->WriteSector(selfSector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    PersistentIntmap* freeMap = new PersistentIntmap();
    freeMap->FetchFrom();

    int n = offset / SectorSize;
    int sector = firstSector;
    for (int i = 0; i < n; i++)
    {
        sector = freeMap->Get(sector);
    }
    delete freeMap;
    return(sector);
}

//----------------------------------------------------------------------
// FileHeader::FileLimit
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLimit()
{
    return limit;
}

int
FileHeader::FileCapacity()
{
    return capacity;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    cout << "File name:" << name << " First Secotr:" << firstSector << " File size:" << limit << " File path:" << filepath << endl;
}
