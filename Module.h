#ifndef __MEM_MODULE_H__
#define __MEM_MODULE_H__

/** INCLUDES **/
#include <iostream>
#include <string>
#include <windows.h>
#include <TlHelp32.h>

/** CLASSES **/

/* MemMod */
class MemMod
{
public:
	MemMod(HANDLE process_handle, MODULEENTRY32 module_entry);
	~MemMod();

	std::wstring get_module_name() const;
	DWORD get_base_addr() const;
	unsigned int get_module_size() const;

	void debug();

private:
	HANDLE process_handle;
	TCHAR module_name[MAX_MODULE_NAME32 + 1];
	DWORD base_addr;
	unsigned int module_size;
};

#endif