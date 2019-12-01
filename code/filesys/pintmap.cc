// pbitmap.c 
//	Routines to manage a persistent bitmap -- a bitmap that is
//	stored on disk.
//
// Copyright (c) 1992,1993,1995 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "pintmap.h"

//----------------------------------------------------------------------
// PersistentIntmap::PersistentIntmap(int)
// 	Initialize a bitmap with "numItems" bits, so that every bit is clear.
//	it can be added somewhere on a list.
//
//	"numItems" is the number of bits in the bitmap.
//
//      This constructor does not initialize the bitmap from a disk file
//----------------------------------------------------------------------

PersistentIntmap::PersistentIntmap(int numItems):IntMap(numItems) 
{ 
}

//----------------------------------------------------------------------
// PersistentIntmap::PersistentIntmap(OpenFile*,int)
// 	Initialize a persistent bitmap with "numItems" bits,
//      so that every bit is clear.
//
//	"numItems" is the number of bits in the bitmap.
//      "file" refers to an open file containing the bitmap (written
//        by a previous call to PersistentIntmap::WriteBack
//
//      This constructor initializes the bitmap from a disk file
//----------------------------------------------------------------------

PersistentIntmap::PersistentIntmap(OpenFile *file, int numItems):IntMap(numItems) 
{ 
    // map has already been initialized by the BitMap constructor,
    // but we will just overwrite that with the contents of the
    // map found in the file
    file->ReadAt((char *)map, numItems * sizeof(int), 0);
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
//
//	"file" is the place to read the bitmap from
//----------------------------------------------------------------------

void
PersistentIntmap::FetchFrom(OpenFile *file) 
{
    file->ReadAt((char *)map, numItems * sizeof(int), 0);
}

//----------------------------------------------------------------------
// PersistentIntmap::WriteBack
// 	Store the contents of a persistent bitmap to a Nachos file.
//
//	"file" is the place to write the bitmap to
//----------------------------------------------------------------------

void
PersistentIntmap::WriteBack(OpenFile *file)
{
   file->WriteAt((char *)map, numItems * sizeof(int), 0);
}
