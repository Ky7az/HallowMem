#ifndef __MEM_PROCESS_H__
#define __MEM_PROCESS_H__

/** INCLUDES **/
#include <iostream>
#include <iomanip>
#include <string>
#include <windows.h>
#include <TlHelp32.h>
#include "..\..\HallowLib\Queue.h"
#include "Module.h"
#include "Chunk.h"

/** DEFINES **/
#define WRITABLE (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)

/** CLASSES **/

/* MemProc */
class MemProc
{
public:
	MemProc(std::wstring process_name, std::wstring window_name);
	~MemProc();

	HANDLE get_process_handle() const;
	Queue* get_mem_chunks() const;

	void scan();
	MemMod* module(std::wstring module_name);
	void read(DWORD addr, void *buffer, int size);
	void write(DWORD addr, void *buffer, int size);
	void debug();

private:
	std::wstring process_name;
	DWORD process_id;
	HANDLE process_handle;
	std::wstring window_name;
	HWND window_handle;
	DWORD base_addr;
	Queue *mem_modules;
	Queue *mem_chunks;
};

/** FUNCTIONS **/
void process_read(HANDLE process_handle, DWORD addr, void *buffer, int size);
void process_write(HANDLE process_handle, DWORD addr, void *buffer, int size);
DWORD get_pid_by_window_name(std::wstring window_name);
DWORD get_pid_by_proc_name(std::wstring process_name);
DWORD get_base_addr(std::wstring module_name);
DWORD get_ptr_by_offsets(HANDLE process_handle, DWORD base_addr, unsigned int offsets[]);
BOOL inject_dll(DWORD process_id, std::wstring dll_path);
BOOL eject_dll(DWORD process_id, std::wstring dll_path);

#endif