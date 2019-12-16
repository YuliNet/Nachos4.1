// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include "debug.h"

//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------
Directory::Directory()
{
    table = NULL;
    tableSize = 0;
}

Directory::Directory(int size)
{
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
	table[i].inUse = FALSE;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

// void
// Directory::FetchFrom(OpenFile *file)
// {
//     (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
// }

void
Directory::FetchFrom(OpenFile* file)
{
    (void) file->ReadAt((char*)&tableSize, sizeof(int), 0);
    table = new DirectoryEntry[tableSize];
    (void) file->ReadAt((char*)table, tableSize * sizeof(DirectoryEntry), sizeof(int));
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

// void
// Directory::WriteBack(OpenFile *file)
// {
//     (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
// }

void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char*)&tableSize, sizeof(int), 0);
    (void) file->WriteAt((char*)table, tableSize * sizeof(DirectoryEntry), sizeof(int));
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen)) //这里的strncmp可以改为strcmp吗？
	    return i;
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
	return table[i].sector;
    return -1;
}

int
Directory::FindWithFullPath(char* name)
{
    if (name[0] != '/')
    {
        DEBUG(dbgFile, "name must start with /");
        return -1;
    }
    
    int i;
    for (i = 1; i < strlen(name) && name[i] != '/'; i++);

    char filename[FileNameMaxLen + 1];
    strncpy(filename, &name[1], FileNameMaxLen);
    if (i <= FileNameMaxLen)
    {
        filename[i] = '\0';

    }
    else
    {
        filename[FileNameMaxLen] = '\0';
    }

    int sector = Find(filename);
    if (sector == -1)return -1;
    else if(i == strlen(name))return sector;

    //检查file是否为目录文件
    FileHeader* header = new FileHeader;
    header->FetchFrom(sector);
    if (header->GetFileType() != TYPE_DIR)
    {
        DEBUG(dbgFile, filename << "is not a directory");
        return -1;
    }
    delete header;

    //递归进入子目录中查找
    OpenFile* subDirFile = new OpenFile(sector);
    Directory* subDir = new Directory;
    subDir->FetchFrom(subDirFile);
    int res = subDir->FindWithFullPath(&name[i]);
    
    delete subDirFile;
    delete subDir;
    return res;
    
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{ 
    if (FindIndex(name) != -1)
	return FALSE;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = TRUE;
            strncpy(table[i].name, name, FileNameMaxLen); 
            table[i].sector = newSector;
        return TRUE;
	}
    DirectoryEntry* newtable = new DirectoryEntry[tableSize << 1];
    for (int i = 0; i < tableSize; i++)
    {
        newtable[i].sector = table[i].sector;
        strncpy(newtable[i].name, table[i].name, FileNameMaxLen);
        newtable[i].inUse = TRUE;
    }

    newtable[tableSize].sector = newSector;
    strncpy(newtable[tableSize].name, name, FileNameMaxLen);
    newtable[tableSize].inUse = TRUE;
    tableSize <<= 1;

    delete table;
    table = newtable;

    return TRUE;	// no space.  Fix when we have extensible files.
}

bool
Directory::AddWithFullPath(char* name,char* filepath, int newSector)
{
    int sector = FindWithFullPath(filepath);
    if (sector != -1)return FALSE;

    OpenFile* file = new OpenFile(sector);
    Directory* dir = new Directory;
    dir->FetchFrom(file);
    bool res = dir->Add(name, newSector);
    
    delete file;
    delete dir;
    return res;
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//TODO:当超过半数的entry没有用时压缩空间
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	return FALSE; 		// name not in directory
    table[i].inUse = FALSE;
    return TRUE;	
}

bool
Directory::RemoveWithFullPath(char* name, char* filepath)
{
    int sector = FindWithFullPath(filepath);
    if (sector == -1)return FALSE;
    
    OpenFile* file = new OpenFile(sector);
    Directory* dir = new Directory;
    dir->FetchFrom(file);
    bool res = dir->Remove(name);

    delete file;
    delete dir;
    return res;
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
    for (int i = 0; i < tableSize; i++)
	    if (table[i].inUse)
	        printf("%s\n", table[i].name);
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    FileHeader *hdr = new FileHeader;
    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
	    printf("Name: %s, FileHeaderSector: %d\n", table[i].name,table[i].sector);
	    hdr->FetchFrom(table[i].sector);
	    hdr->Print();
        if (hdr->GetFileType() == TYPE_DIR)
        {
            Directory* subdir = new Directory();
            OpenFile* subdirfile = new OpenFile(table[i].sector);
            subdir->FetchFrom(subdirfile);
            subdir->Print();
            delete subdir;
            delete subdirfile;
        }
	}
    printf("\n");
    delete hdr;
}