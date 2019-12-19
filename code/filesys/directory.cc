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

Directory::Directory()
{
    tableSize = 10;
    table = new DirectoryEntry[tableSize];
    for (int i = 0; i < tableSize; i++)
	table[i].inUse = FALSE;
}
//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

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

void
Directory::FetchFrom(OpenFile *file)
{
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
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
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
	    return i;
    return -1;		// name not in directory
}

//在当前目录中查找name，如果找到返回其fileHeader扇区号，否则返回-1
int
Directory::dirlookup(char* name)
{
    int k = -1;
    for (int i = 0; i < tableSize; i++)
    {
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
        {
            k = i;
            break;
        }
    }

    if (k != -1)
    {
        return table[k].sector;
    }
    return -1;
}

// 源自XV6
// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
char*
Directory::skipelem(char* path, char* name)
{
    char *s;
    int len;

    //跳过开头的‘/’
    while(*path == '/')
        path++;
    if(*path == 0)
        return 0;
    s = path;
    //找下一个'/',两个'/'之间为一层目录
    while(*path != '/' && *path != 0)
        path++;
    len = path - s;
    if(len >= FileNameMaxLen)
        memmove(name, s, FileNameMaxLen);
    else 
    {
        memmove(name, s, len);
        name[len] = 0;
    }
    while(*path == '/')
        path++;
    return path;
}


// 从当前目录开始，递归查找文件name
// 如果查找成功，返回其fileHeader sectornum，否则返回-1
// 如果nameparent == 1,返回name所在目录的fileHeader sectorNum
int
Directory::namex(char* path, int nameparent, char* name)
{
    int next;

    path = skipelem(path, name);
    //递归基，相当于在当前目录中查找一个普通文件
    if (*path == '\0')
    {
        if (nameparent) return sector;
        else 
            return dirlookup(name);
    }

    if ((next = dirlookup(name)) == -1)
    {
        return -1;
    }

    FileHeader* hdr = new FileHeader;
    hdr->FetchFrom(next);
    if (hdr->getFileType() != TYPE_DIR)
    {
        delete hdr;
        return -1;
    }
    delete hdr;

    OpenFile* nextDirOpenFile = new OpenFile(next);
    Directory* nextDir = new Directory();
    nextDir->setSector(next);
    nextDir->FetchFrom(nextDirOpenFile);
    delete nextDirOpenFile;

    int res =  nextDir->namex(path, nameparent, name);
    delete nextDir;
    return res;
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
Directory::Find(char *path)
{
    char name[FileNameMaxLen];
    return namex(path, 0, name);
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
Directory::AddInCurrentDir(char* name, int newSector)
{
    if (FindIndex(name) != -1)
        return FALSE;

    for (int i = 0; i < tableSize; i++)
    if (!table[i].inUse) 
    {
        table[i].inUse = TRUE;
        strncpy(table[i].name, name, FileNameMaxLen);
        table[i].sector = newSector;
        return TRUE;
    }
    //TODO:increase
    return FALSE;
}

bool
Directory::Add(char *path, int newSector)
{
    char name[FileNameMaxLen];
    int psector = namex(path, 1, name);
    
    if (psector == -1)
        return FALSE;
    Directory* pdir = new Directory();
    OpenFile* pfile = new OpenFile(psector);
    pdir->FetchFrom(pfile);
    
    int res = pdir->AddInCurrentDir(name, newSector);
    if (res)
    {
        // cout << "*********";
        // pdir->Print();
        // cout << "*********";

        pdir->WriteBack(pfile);
    }
    delete pfile;
    delete pdir;
    return res;
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------
bool
Directory::RemoveInCurrentDir(char* name)
{
    int index = FindIndex(name);
    if (index == -1)return FALSE;
    table[index].inUse = FALSE;
    return TRUE;
}


bool
Directory::Remove(char *path)
{ 
    char name[FileNameMaxLen];
    int psector = namex(path, 1, name);
    if (psector == -1)
        return FALSE;
    
    Directory* pdir = new Directory();
    OpenFile* pfile = new OpenFile(psector);
    pdir->FetchFrom(pfile);

    int res = pdir->RemoveInCurrentDir(name);
    if (res)
    {
        pdir->WriteBack(pfile);
    }
    delete pfile;
    delete pdir;
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
	    printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
	    // hdr->FetchFrom(table[i].sector);
	    // hdr->Print();
	}
    printf("\n");
    delete hdr;
}

void
Directory::selfTest()
{
    

}