// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#ifndef FILESYS_STUB

#include "copyright.h"
#include "debug.h"
#include "disk.h"
#include "pbitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		0
#define DirectorySector 	1
#define PipeSector          2

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize 	(NumSectors / BitsInByte)
#define NumDirEntries 		10
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries)
#define PipeFileSize        1024

//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{ 
    DEBUG(dbgFile, "Initializing the file system.");
    if (format) {
        PersistentBitmap *freeMap = new PersistentBitmap(NumSectors);
        Directory *directory = new Directory(DirectorySector, NumDirEntries);
	FileHeader *mapHdr = new FileHeader(FreeMapSector, TYPE_FILE);
	FileHeader *dirHdr = new FileHeader(DirectorySector, TYPE_DIR);
    FileHeader* pipeHdr = new FileHeader(PipeSector, TYPE_PIPE);

        DEBUG(dbgFile, "Formatting the file system.");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);
    freeMap->Mark(PipeSector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

	ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));
    ASSERT(pipeHdr->Allocate(freeMap, PipeFileSize));

    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

        DEBUG(dbgFile, "Writing headers back to disk.");
	mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);
    pipeHdr->WriteBack(PipeSector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG(dbgFile, "Writing bitmap and directory back to disk.");
	freeMap->WriteBack(freeMapFile);	 // flush changes to disk
	directory->WriteBack(directoryFile);

	if (debug->IsEnabled('f')) {
	    freeMap->Print();
	    directory->Print();
        selfTest();
        }
        delete freeMap; 
	delete directory; 
	delete mapHdr; 
	delete dirHdr;
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
    }
}

//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(char *name, FileType type)
{
    Directory *directory;
    PersistentBitmap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG(dbgFile, "Creating file " << name << " type " << type);

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1)
    {
        success = FALSE;			// file is already in directory
    }
    else 
    {
        freeMap = new PersistentBitmap(freeMapFile,NumSectors);
        sector = freeMap->FindAndSet();	// find a sector to hold the file header
        if (sector == -1)
        {
            success = FALSE;		// no free block for file header
        }
        else if (!directory->Add(name, sector))
        {
            success = FALSE;	// no space in directory
        }
        else 
        {
            hdr = new FileHeader(sector, type);
            if (type == TYPE_DIR)
            {
                success = hdr->Allocate(freeMap, DirectoryFileSize);
                hdr->WriteBack(sector); //要先把hdr写回磁盘，因为newdir写回磁盘时会查hdr
                Directory* newdir = new Directory(sector, NumDirEntries);
                OpenFile* newfile = new OpenFile(sector);
                newdir->WriteBack(newfile);
                delete newdir;
                delete newfile;
            }
            
            else if (type == TYPE_FILE)
            {
                hdr->WriteBack(sector);
                success = TRUE;
            }
            freeMap->WriteBack(freeMapFile);
            delete hdr;
        }

        delete freeMap;
    }
    delete directory;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(char *name)
{ 
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG(dbgFile, "Opening file" << name);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name); 
    if (sector >= 0) 		
	openFile = new OpenFile(sector);	// name was found in directory 
    delete directory;
    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(char *name)
{ 
    Directory *directory;
    PersistentBitmap *freeMap;
    FileHeader *fileHdr;
    int sector;
    
    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return FALSE;			 // file not found 
    }
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new PersistentBitmap(freeMapFile,NumSectors);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List()
{
    Directory *directory = new Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    PersistentBitmap *freeMap = new PersistentBitmap(freeMapFile,NumSectors);
    Directory *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
}

int
FileSystem::ReadPipe(char* data)
{
    FileHeader* pipehdr = new FileHeader();
    pipehdr->FetchFrom(PipeSector);
    OpenFile* pipefile = new OpenFile(PipeSector);
    int res = pipefile->ReadAt(data, pipehdr->FileLimit(), 0);
    delete pipehdr;
    delete pipefile;
    return res;
}

void
FileSystem::WritePipe(char* data, int length)
{
    OpenFile* pipefile = new OpenFile(PipeSector);
    pipefile->WriteAt(data, length, 0);
    delete pipefile;
}

void
FileSystem::selfTest()
{
    cout << "FileSystem Test" << endl;
    
    // cout << "FileHeader test" << endl;
    // testFileHdr->setSelfSector(testFileSector);
    // testFileHdr->selfTest(freeMap);
    // testFileHdr->WriteBack(testFileSector);
    
    
    // 初始环境
    // FileHeader* testFileHdr = new FileHeader;
    // PersistentBitmap *freeMap = new PersistentBitmap(freeMapFile,NumSectors);
    // freeMap->Print();
    // int testFileSector = freeMap->FindAndSet();
    // testFileHdr->setSelfSector(testFileSector);
    // testFileHdr->FetchFrom(testFileSector);
    // testFileHdr->Deallocate(freeMap);
    // testFileHdr->WriteBack(testFileSector);
    // freeMap->WriteBack(freeMapFile);

    // cout << "OpenFile Test" << endl;
    
    // OpenFile* testFile = new OpenFile(testFileSector);
    // testFile->selfTest();

    //Directory test
    // PersistentBitmap* freeMap = new PersistentBitmap(NumSectors);
    // freeMap->FetchFrom(freeMapFile);
    // Directory* root = new Directory();
    // root->FetchFrom(directoryFile);
    // root->setSector(DirectorySector);
    // cout << "初始环境：" << endl;
    // freeMap->Print();
    // root->Print();
    // cout << "=================" << endl;

    // cout << "在根目录下创建目录a" << endl;
    // int asector = freeMap->FindAndSet();
    // FileHeader* aHdr = new FileHeader;
    // ASSERT(aHdr->Allocate(freeMap, DirectoryFileSize));
    // aHdr->setSelfSector(asector);
    // aHdr->setFileType(TYPE_DIR);
    // ASSERT(root->Add("/a", asector));
    // freeMap->Print();
    // root->FetchFrom(directoryFile);
    // root->Print();
    // aHdr->WriteBack(asector);
    // // freeMap->WriteBack(freeMapFile);
    // cout << "=================" << endl;


    // cout << "在目录a下创建文件b" << endl;
    // Directory* a = new Directory();
    // OpenFile* afile = new OpenFile(asector);
    // a->WriteBack(afile);    //清空原始扇区
    // int bSector = freeMap->FindAndSet();
    // FileHeader* bHdr = new FileHeader;
    // bHdr->setSelfSector(bSector);
    // bHdr->setFileType(TYPE_FILE);
    // ASSERT(root->Add("/a/b", bSector));
    // freeMap->Print();
    // root->Print();
    // a->FetchFrom(afile);
    // a->Print();
    // cout << "=================" << endl;

    // cout << "删除a目录下的文件b" << endl;
    // root->Remove("a/b");
    // a->FetchFrom(afile);
    // a->Print();



    // filesys test
    // ASSERT(Create("/a", TYPE_DIR));
    // ASSERT(Create("/c", TYPE_FILE));
    // Print();
    // ASSERT(Create("/a/b", TYPE_FILE));
    // Print();
    // // OpenFile* b = Open("/a/b");
    // // b->selfTest();
    // ASSERT(Remove("/a/b"));
    // Print();
    // ASSERT(Remove("/c"));
    // Print();


    // pipe test
    char str[SectorSize+1];
    cout << "input : " << endl;
    cin >> str;
    WritePipe(str, strlen(str));

    cout << "output : " << endl;
    char str2[SectorSize+1];
    int length = ReadPipe(str2);
    cout << str2 << endl;

}

#endif // FILESYS_STUB
