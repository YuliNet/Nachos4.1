/*
 * @Author: Lollipop
 * @Date: 2019-11-03 21:19:35
 * @LastEditors: Lollipop
 * @LastEditTime: 2019-11-15 13:47:41
 * @Description: 
 */
// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "noff.h"
#include "translate.h"
#include "machine.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(int threadId, char* fileName);
    ~AddrSpace();			// De-allocate an address space

    void Execute();             	// Run a program
					// assumes the program has already
                                        // been loaded

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch

    TranslationEntry* getPageTable() {return pageTable;}
    int getNumPages() {return numPages;}

    OpenFile* getExeFileId() {return exeFileId;}

    // Translate virtual address _vaddr_
    // to physical address _paddr_. _mode_
    // is 0 for Read, 1 for Write.
    ExceptionType Translate(unsigned int vaddr, unsigned int *paddr, int mode);

  private:
    TranslationEntry *pageTable;

    int threadId;
    unsigned int numPages;		// Number of pages in the virtual address space
    OpenFile* exeFileId;
    
    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

};

#endif // ADDRSPACE_H
