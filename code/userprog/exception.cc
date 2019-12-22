// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "directory.h"

void FileSystemHandler(int type);
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    // DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
      switch(type) {
      case SC_Halt:
	DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

	SysHalt();

	ASSERTNOTREACHED();
	break;

      case SC_Add:
	DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
	
	/* Process SysAdd Systemcall*/
	int result;
	result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));

	DEBUG(dbgSys, "Add returning with " << result << "\n");
	/* Prepare Result */
	kernel->machine->WriteRegister(2, (int)result);
	
	/* Modify return point */
	kernel->machine->PCAdvanced();

	return;
	
	ASSERTNOTREACHED();

	break;

	case SC_Create :
	case SC_Remove :
	case SC_Open :
	case SC_Read :
	case SC_Write :
	case SC_Seek :
	case SC_Close :
		FileSystemHandler(type);
		return;
		ASSERTNOTREACHED();
		break;

      default:
	cerr << "Unexpected system call " << type << "\n";
	break;
      }
      break;
    default:
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}

// Helper function to get file name using ReadMem for Create and Open syscall
char* getFileNameFromAddress(int address) {
    int position = 0;
    int data;
    char* name = new char[FileNameMaxLen + 1];
    do {
        // each time read one byte
        bool success = kernel->machine->ReadMem(address + position, 1, &data);
        // ASSERT_MSG(success, "Fail to read memory in Create syscall");
        name[position++] = (char)data;

        // ASSERT_MSG(position <= FileNameMaxLength, "Filename length too long")
    } while(data != '\0');
    name[position] = '\0';
    return name;
}

void FileSystemHandler(int type)
{
	if (type == SC_Create)
	{
		int address = kernel->machine->ReadRegister(4);
		char* name = getFileNameFromAddress(address);
		bool success = kernel->fileSystem->Create(name, 1024);
		if (success)
		{
			DEBUG(dbgSys, "File " << name << " created.");
		}
		else
		{
			DEBUG(dbgSys, "File " << name << " fail to create.");
		}
		kernel->machine->WriteRegister(2, (int)success);
	}
	else if (type == SC_Open)
	{
		int address = kernel->machine->ReadRegister(4);
		char* name = getFileNameFromAddress(address);
		OpenFile* openfile = kernel->fileSystem->Open(name);
		DEBUG(dbgSys, "File " << name << " opened.");
		kernel->machine->WriteRegister(2, (int)openfile);
	}
	else if(type == SC_Read)
	{
		int position = kernel->machine->ReadRegister(4);
		int numBytes = kernel->machine->ReadRegister(5);
		int openfileId = kernel->machine->ReadRegister(6);

		OpenFile* openfile = (OpenFile*) openfileId;
		char buffer[numBytes];
		int result = openfile->Read(buffer, numBytes);
		for (int i = 0; i < result; i++)
		{
			kernel->machine->WriteMem(position+i, 1, int(buffer[i]));
		}
		DEBUG(dbgSys, "Read " << result << " bytes into buffer.");
		kernel->machine->WriteRegister(2, result);
	}
	else if(type == SC_Write)
	{
		int position = kernel->machine->ReadRegister(4);
		int numBytes = kernel->machine->ReadRegister(5);
		int openfileId = kernel->machine->ReadRegister(6);
		int data;
		char buffer[numBytes];
		for (int i = 0; i < numBytes; i++)
		{
			kernel->machine->ReadMem(position+i, 1, &data);
			buffer[i] = (char)data;
		}

		OpenFile* openfile = (OpenFile*) openfileId;
		int result = openfile->Write(buffer, numBytes);
		DEBUG(dbgSys, "Write " << result << " bytes into file.");
		kernel->machine->WriteRegister(2, result);
	}
	else if(type == SC_Close)
	{
		int openfileId = kernel->machine->ReadRegister(4);
		OpenFile* openfile = (OpenFile*) openfileId;
		delete openfile;
		//TODO:判断close 失败情况
		DEBUG(dbgSys, "File has closed.");
		kernel->machine->WriteRegister(2, 1);
	}
	else if(type == SC_Remove)
	{
		int address = kernel->machine->ReadRegister(4);
		char* name = getFileNameFromAddress(address);
		bool success = kernel->fileSystem->Remove(name);
		if (success)
		{
			DEBUG(dbgSys, "File " << name << " has removed.");
		}
		else
		{
			DEBUG(dbgSys, "File " << name << " fail to remove.");
		}
		kernel->machine->WriteRegister(2, (int)success);
	}

	else
	{
		cerr << "no handler" << endl;
	}
	kernel->machine->PCAdvanced();
}