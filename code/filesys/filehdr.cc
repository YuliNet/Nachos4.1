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

FileHeader::FileHeader()
{
    limit = 0;
    capacity = 0;
    nsectors = 0;
    memset(dataSectors, 0, sizeof(dataSectors));
    createTime = kernel->stats->totalTicks;
}

FileHeader::FileHeader(int selfSector, FileType type)
{
    limit = 0;
    capacity = 0;
    nsectors = 0;
    this->selfSector = selfSector;
    this->type = type;
    memset(dataSectors, 0, sizeof(dataSectors));
    createTime = kernel->stats->totalTicks;
}

//----------------------------------------------------------------------
//申请fileSize个字节(设计为在文件空间不够时追加申请，在刚完成文件创建时不用再指定文件初始大小，一律为0)
//----------------------------------------------------------------------

bool FileHeader::Allocate(PersistentBitmap *freeMap, int size)
{
    //边界情况处理
    if (size == 0)return TRUE;
    if (size < 0 || (size + capacity) > MaxFileSize)
        return FALSE;

    int n = divRoundUp(size, SectorSize);
    const int N = n;
    if (freeMap->NumClear() < n)
        return FALSE; // not enough space

    //直接索引
    for (int i = nsectors; (i < NumDirect) && (n > 0); i++)
    {
        // freeMap->Print();
        dataSectors[i] = freeMap->FindAndSet();
        // cout << "***********" << endl;
        // freeMap->Print();
        // cout << dataSectors[i] << endl;
        ASSERT(dataSectors[i] > 0);
        n--;
    }
    if (n == 0)
    {
        nsectors += N;
        capacity = nsectors * SectorSize;
        // modifyTime = kernel->stats->totalTicks;
        return TRUE;
    }

    int inDirect[NumInDirect] = {0};
    //如果还没有间接块，申请一个间接块
    if (dataSectors[NumDirect] == 0)
    {
        dataSectors[NumDirect] = freeMap->FindAndSet();
        ASSERT(dataSectors[NumDirect] >= 0);
        //初始化间接块
        kernel->synchDisk->WriteSector(dataSectors[NumDirect], (char *)inDirect);
    }

    kernel->synchDisk->ReadSector(dataSectors[NumDirect], (char *)inDirect);

    //填间接块
    for (int i = 0; (i < NumInDirect) && (n > 0); i++)
    {
        if (inDirect[i] == 0)
        {
            inDirect[i] = freeMap->FindAndSet();
            ASSERT(inDirect[i] >= 0);
            n--;
        }
    }

    if (n == 0)
    {
        nsectors += N;
        capacity = nsectors * SectorSize;
        kernel->synchDisk->WriteSector(dataSectors[NumDirect], (char *)inDirect);
        // modifyTime = kernel->stats->totalTicks;
        return TRUE;
    }
    return FALSE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void FileHeader::Deallocate(PersistentBitmap *freeMap)
{
    //回收直接块
    for (int i = 0; i < nsectors && i < NumDirect; i++)
    {
        if (freeMap->Test(dataSectors[i]))
        {
            freeMap->Clear(dataSectors[i]);
            dataSectors[i] = 0;
        }
            
    }

    //回收间接块
    if (nsectors > NumDirect)
    {
        int inDirect[NumInDirect];
        kernel->synchDisk->ReadSector(dataSectors[NumDirect], (char *)inDirect);
        for (int i = 0; i < NumInDirect; i++)
        {
            if (freeMap->Test(inDirect[i]))
                freeMap->Clear(inDirect[i]);
        }
        freeMap->Clear(dataSectors[NumDirect]);
    }
    dataSectors[NumDirect] = 0;
    limit = 0;
    capacity = 0;
    nsectors = 0;
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk.
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void FileHeader::FetchFrom(int sector)
{
    kernel->synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk.
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void FileHeader::WriteBack(int sector)
{
    kernel->synchDisk->WriteSector(sector, (char *)this);
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

int FileHeader::ByteToSector(int offset)
{
    int n = offset / SectorSize;
    if (n < NumDirect)
    {
        ASSERT(dataSectors[n] > 0);
        return dataSectors[n];
    }

    ASSERT(dataSectors[NumDirect] > 0);
    n -= NumDirect;
    int inDirect[NumInDirect] = {0};
    kernel->synchDisk->ReadSector(dataSectors[NumDirect], (char *)inDirect);

    ASSERT(inDirect[n] > 0);
    return inDirect[n];
}

int FileHeader::FileLimit()
{
    return limit;
}

int FileHeader::FileCapacity()
{
    return capacity;
}

bool
FileHeader::setFileLimit(int position)
{
    if (position < 0 || position > capacity)return FALSE;
    limit = position;
    return TRUE;
}
//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File limit: %d.  File capacity: %d.  File nSectors: %d.\n", limit, capacity, nsectors);
    cout << "File Directblocks:" << endl;
    for (i = 0; i < nsectors && i < NumDirect; i++)
    {
        if (dataSectors[i] > 0)
            cout << dataSectors[i] << ", ";
    }
    cout << endl;
    if (nsectors > NumDirect)
    {
        cout << "File InDirectblocks:" << endl;
        int inDirect[NumInDirect] = {0};
        kernel->synchDisk->ReadSector(dataSectors[NumDirect], (char *)inDirect);
        for (int i = 0; i < NumInDirect; i++)
        {
            if (inDirect[i] != 0)
            {
                cout << inDirect[i] << " ";
            }
        }
        cout << endl;
    }
    // printf("\nFile contents:\n");
    // for (i = k = 0; i < nsectors; i++) {
    // kernel->synchDisk->ReadSector(dataSectors[i], data);
    //     for (j = 0; (j < SectorSize) && (k < limit); j++, k++) {
    //     if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
    // 	printf("%c", data[j]);
    //         else
    // 	printf("\\%x", (unsigned char)data[j]);
    // }
    //     printf("\n");
    // }
    delete[] data;
}

void FileHeader::selfTest(PersistentBitmap *freeMap)
{
    cout << "FileHeader selftest begin" << endl;
    freeMap->Print();
    Print();
    cout << "边界情况测试：" << endl;
    Allocate(freeMap, -1);
    Allocate(freeMap, 0);
    Allocate(freeMap, MaxFileSize + 1);
    freeMap->Print();
    Print();
    cout << "正常情况Allocate测试：" << endl;
    cout << "Allocate 1024 bytes" << endl;
    Allocate(freeMap, 1024);
    freeMap->Print();
    Print();
    cout << "Allocate 5120 bytes" << endl;
    Allocate(freeMap, 5120);
    freeMap->Print();
    Print();
    cout << "Deallocate" << endl;
    Deallocate(freeMap);
    freeMap->Print();
    Print();
}
