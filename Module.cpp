#include "Module.h"
using namespace std;

MemMod::MemMod(HANDLE process_handle, MODULEENTRY32 module_entry)
{
	this->process_handle = process_handle;
	memset(module_name, 0, wcslen(module_entry.szModule) + 1);
	wcsncpy_s(module_name, module_entry.szModule, wcslen(module_entry.szModule));
	this->base_addr = (DWORD)module_entry.hModule;
	this->module_size = module_entry.modBaseSize;
}

MemMod::~MemMod()
{
}

wstring MemMod::get_module_name() const { return module_name; }
DWORD MemMod::get_base_addr() const { return base_addr; }
unsigned int MemMod::get_module_size() const { return module_size; }

void MemMod::debug()
{
	wcout << module_name << " 0x" << hex << base_addr << dec;
	wcout << " <" << module_size << " bytes>" << endl;
}