#ifndef MEMORY_H
#define MEMORY_H
// Definitions related to the size, and format of user memory

const int PageSize = 128; 		// set the page size equal to
					// the disk sector size, for simplicity

//
// You are allowed to change this value.
// Doing so will change the number of pages of physical memory
// available on the simulated machine.
//
const int NumPhysPages = 128;
const int NumPhysPagesPerThread = 8;
const int NumVirtualPages = 1024;

const int PhysicalMemorySize = (NumPhysPages * PageSize);
const int VirtualMemorySize = (NumVirtualPages * PageSize);

#endif