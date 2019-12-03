// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "pintmap.h"
#include "time.h"

typedef time_t TimeWrapper;
typedef enum
{
	TYPE_FILE,
	TYPE_DIR,
	TYPE_UNKNOWN
}FileType;

#define FileNameMaxLen 		9	// for simplicity, we assume 
					// file names are <= 9 characters long
#define FilePathMaxLen     100

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
    bool Allocate(PersistentIntmap *intMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(PersistentIntmap *intMap);  // De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk
    void WriteBack();

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

    bool AllocateMemory(int numSectors);

    void SetSelfSector(int sector) {selfSector = sector;};
    void SetNumBytes(int n) {numBytes = n;};
    void SetFileType(FileType type) {this->type = type;};
    FileType GetFileType() {return this->type;};
    

  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file

    FileType type;

    TimeWrapper createTime;
    TimeWrapper accessTime;
    TimeWrapper modifyTime;

    int selfSector;
    int firstSector;
    int lastSector;

    char name[FileNameMaxLen + 1];
    char filepath[FilePathMaxLen + 1];
};

#endif // FILEHDR_H
