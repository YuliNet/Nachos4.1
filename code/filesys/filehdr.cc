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
    numBytes = 0;
    numSectors = 0;
    this->type = type;
    this->selfSector = sector;
    strcpy(name, filename);
    strcpy(this->filepath, filepath);
}


//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(int fileSize)
{ 
    PersistentIntmap* freeMap = new PersistentIntmap();
    freeMap->FetchFrom();

    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	    return FALSE;		// not enough space

    firstSector = freeMap->FindAndSet();
    int pre = firstSector;
    for (int i = 1; i < numSectors; i++)
    {
        int now = freeMap->FindAndSet();
        freeMap->Mark(pre, now);
        pre= now;
    }
    lastSector = pre;
    freeMap->WriteBack();
    delete freeMap;
    return TRUE;
}

bool
FileHeader::AllocateMemory(int numSectors)
{
    PersistentIntmap *freeMap = new PersistentIntmap();
    freeMap->FetchFrom();

    if (freeMap->NumClear() < numSectors)
        return FALSE;
    
    for (int i = 0; i < numSectors; i++)
    {
        int next = freeMap->FindAndSet();
        freeMap->Mark(lastSector, next);
        lastSector = next;
    }
    numSectors += numSectors;
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
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    cout << "File name : " << name << "\tFirst Secotr : " << firstSector << "\tFile size : " << numBytes << "\tbytes " << "File path : " << filepath << endl;
}
