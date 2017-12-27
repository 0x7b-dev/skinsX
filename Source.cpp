/*
	 HERE ARE EXAMPLES OF HOW TO USE DIFFERENT FUNCTIONS PROVIDED IN THE CLASS nbqmemory.h
*/

#include "nbqmemory.h"
#include <iostream>

using namespace std;


int main()
{
	NBQMemory mem; // initialization of the class

	// retrieves handle to a process (eg. notepad.exe) with full access rights
	HANDLE handle1 = mem.GetHandleByProcessName("notepad.exe", PROCESS_ALL_ACCESS);

	// retrieves handle to a process with certain window title (eg. Calculator) with read/write rights
	HANDLE handle2 = mem.GetHandleByWindowName("Calculator", PROCESS_VM_READ | PROCESS_VM_WRITE);

	// retrieves process id of a process (eg. notepad.exe)
	DWORD pID1 = mem.GetProcessIdByProcessName("notepad.exe");

	// retrieves process id of a process with certain window title (eg. Calculator)
	DWORD pID2 = mem.GetProcessIdByWindowName("Calculator");

	// returns the base address of a specified module (eg. process id of csgo.exe, client.dll module)
	DWORD modAddress = mem.GetModuleBaseAddress(mem.GetProcessIdByProcessName("csgo.exe"), "client.dll");

	// easy to use wrapper for ReadProcessMemory with template for each datatype
	int x = mem.ReadMemory<int>(handle1, 0xABCDEF); // reads memory the size of an integer
	float y = mem.ReadMemory<float>(handle1, 0xABCDEF); // reads memory the size of an float
	bool z = mem.ReadMemory<bool>(handle1, 0xABCDEF); // reads memory the size of a boolean

	// easy to use wrapper for WriteProcessMemory with template for each datatype
	mem.WriteMemory<int>(handle2, 0xABCDEF, 12345); // writes integer value to certain memory address
	mem.WriteMemory<float>(handle2, 0xABCDEF, 123.45); // writes float value to certain memory address
	mem.WriteMemory<bool>(handle2, 0xABCDEF, true); // writes boolean value to certain memory address

	return 0;
}