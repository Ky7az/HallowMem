#include "Process.h"
using namespace std;

/* MemProc */
MemProc::MemProc(wstring process_name, wstring window_name)
{
	// Process Name
	this->process_name = process_name;
	// Window Name
	this->window_name = window_name;

	// Process Id
	if ((this->process_id = get_pid_by_window_name(window_name)) == 0) {
		this->process_handle = NULL;
		throw exception();
	}

	// Process Handle
	if ((this->process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_id)) == 0) {
		throw exception();
	}

	// Window Handle
	if (window_name.empty()) {
		this->window_handle = FindWindowW(NULL, window_name.c_str());
	}

	// Base Address
	if ((this->base_addr = get_base_addr(process_name)) == 0) {
		this->base_addr = 0x400000;
	}

	this->mem_modules = new Queue();
	this->mem_chunks = new Queue();
}

MemProc::~MemProc()
{
	if (mem_modules != NULL)
		delete mem_modules;

	if (mem_chunks != NULL) {
		delete mem_chunks;
	}
}

HANDLE MemProc::get_process_handle() const { return process_handle; }
Queue* MemProc::get_mem_chunks() const { return mem_chunks; }

void MemProc::scan()
{
	if (process_handle == NULL)
		return;

	// Mem Modules
	MODULEENTRY32 module_entry;
	module_entry.dwSize = sizeof(module_entry);
	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);

	if (h_snapshot == INVALID_HANDLE_VALUE) {
		return;
	}

	if (!Module32First(h_snapshot, &module_entry)) {
		CloseHandle(h_snapshot);
		return;
	}

	while (1) {

		MemMod *mm = new MemMod(process_handle, module_entry);
		mem_modules->add(mm);

		if (Module32Next(h_snapshot, &module_entry) == 0) {
			break;
		}
	}
	CloseHandle(h_snapshot);

	// Mem Chunks
	MEMORY_BASIC_INFORMATION mbi;
	DWORD addr = 0;

	while (1) {

		if (VirtualQueryEx(process_handle, (LPCVOID)addr, &mbi, sizeof(mbi)) == 0) {
			break;
		}
		if ((mbi.State & MEM_COMMIT) && (mbi.Protect & WRITABLE)) {
			MemChunk *mc = new MemChunk(process_handle, &mbi);
			mc->read();
			mem_chunks->add(mc);
		}
		addr = (DWORD)mbi.BaseAddress + mbi.RegionSize;
	}
}

MemMod* MemProc::module(wstring module_name)
{
	Node *n = mem_modules->get_head();
	while (n != NULL) {
		MemMod *mm = (MemMod*)n->get_data();
		if (module_name == mm->get_module_name()) {
			return mm;
		}
		n = n->get_next();
	}
	return NULL;
}

void MemProc::read(DWORD addr, void *buffer, int size)
{
	process_read(process_handle, addr, buffer, size);
}

void MemProc::write(DWORD addr, void *buffer, int size)
{
	process_write(process_handle, addr, buffer, size);
}

void MemProc::debug()
{
	wcout << "Process Name = " << process_name << endl;
	wcout << "Process Id = " << process_id << endl;
	wcout << "Window Name = " << window_name << endl;
	wcout << "Base Addr = 0x" << hex << base_addr << dec << endl;
	wcout << "Modules Count = " << mem_modules->get_len() << endl;
	wcout << "Chunks Count = " << mem_chunks->get_len() << endl;

	/*cout << endl << "# Modules" << endl;
	Node *nm = mem_modules->get_head();
	while (nm != NULL) {
		MemMod *mm = (MemMod*)nm->get_data();
		mm->debug();
		nm = nm->get_next();
	}*/

	/*cout << endl << "# Chunks" << endl;
	Node *nc = mem_chunks->get_head();
	while (nc != NULL) {
		MemChunk *mc = (MemChunk*) nc->get_data();
		mc->debug();
		nc = nc->get_next();
	}*/
}

void process_read(HANDLE process_handle, DWORD addr, void *buffer, int size)
{
	unsigned int bytes_left = size;
	unsigned int total_read = 0;
	unsigned int bytes_to_read;
	DWORD bytes_read;
	static unsigned char tempbuf[128 * 1024];

	while (bytes_left) {
		bytes_to_read = (bytes_left > sizeof(tempbuf)) ? sizeof(tempbuf) : bytes_left;
		ReadProcessMemory(process_handle, (unsigned char*)addr + total_read, tempbuf, bytes_to_read, &bytes_read);
		if (bytes_read != bytes_to_read)
			break;
		memcpy((unsigned char*)buffer + total_read, tempbuf, bytes_read);
		bytes_left -= bytes_read;
		total_read += bytes_read;
	}
}

void process_write(HANDLE process_handle, DWORD addr, void *buffer, int size)
{
	WriteProcessMemory(process_handle, (LPVOID)addr, buffer, size, NULL);
}

DWORD get_pid_by_window_name(wstring window_name)
{
	DWORD dw_pid = 0;
	HWND h_window = FindWindowW(0, window_name.c_str());
	if (h_window != NULL) {
		GetWindowThreadProcessId(h_window, &dw_pid);
	}

	//CloseHandle(h_window);
	return dw_pid;
}

DWORD get_pid_by_proc_name(wstring process_name)
{
	DWORD dw_pid = 0;
	PROCESSENTRY32 pe32;
	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (h_snapshot == INVALID_HANDLE_VALUE)
		return 0;

	if (!Process32First(h_snapshot, &pe32))
		return 0;

	while (Process32Next(h_snapshot, &pe32)) {
		if (!wcscmp(process_name.c_str(), pe32.szExeFile)) {
			dw_pid = pe32.th32ProcessID;
			break;
		}
		Sleep(1);
	}

	CloseHandle(h_snapshot);
	return dw_pid;
}

DWORD get_base_addr(wstring module_name)
{
	MODULEENTRY32 module_entry;
	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, get_pid_by_proc_name(module_name));

	if (!h_snapshot)
		return 0;

	module_entry.dwSize = sizeof(module_entry);
	BOOL b_module = Module32First(h_snapshot, &module_entry);

	while (b_module) {

		if (!wcscmp(module_entry.szModule, module_name.c_str())) {
			CloseHandle(h_snapshot);
			return (DWORD)module_entry.modBaseAddr;
		}

		b_module = Module32Next(h_snapshot, &module_entry);
	}

	CloseHandle(h_snapshot);
	return 0;
}

DWORD get_ptr_by_offsets(HANDLE process_handle, DWORD base_addr, unsigned int offsets[])
{
	DWORD addr;
	for (int i = 0; i <= sizeof(offsets); i++) {
		process_read(process_handle, base_addr, &addr, sizeof(addr));
		base_addr = addr + offsets[i];
	}
	return base_addr;
}

BOOL inject_dll(DWORD process_id, wstring dll_path)
{
	HANDLE h_process, h_thread;
	LPVOID base_addr, func_addr;
	DWORD mem_size, exit_code;
	HMODULE h_kernel_32;
	BOOL success = FALSE;

	if ((h_process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, process_id)))
	{
		mem_size = wcslen(dll_path.c_str()) * 2 + 2;
		if ((base_addr = VirtualAllocEx(h_process, NULL, mem_size, MEM_COMMIT, PAGE_READWRITE)))
		{
			if (WriteProcessMemory(h_process, base_addr, dll_path.c_str(), mem_size, NULL))
			{
				if ((h_kernel_32 = LoadLibraryW(L"kernel32.dll")))
				{
					if ((func_addr = GetProcAddress(h_kernel_32, "LoadLibraryW")))
					{
						if ((h_thread = CreateRemoteThread(h_process, NULL, 0, (LPTHREAD_START_ROUTINE)func_addr, base_addr, 0, NULL)))
						{
							WaitForSingleObject(h_thread, INFINITE);
							if (GetExitCodeThread(h_thread, &exit_code))
							{
								success = (exit_code != 0) ? TRUE : FALSE;
							}
							CloseHandle(h_thread);
						}
					}
					FreeLibrary(h_kernel_32);
				}
			}
			VirtualFreeEx(h_process, base_addr, 0, MEM_RELEASE);
		}
		CloseHandle(h_process);
	}

	return success;
}

BOOL eject_dll(DWORD process_id, wstring dll_path)
{
	HANDLE h_process, h_thread;
	HMODULE h_dll, h_kernel_32;
	LPVOID func_addr, base_addr;
	DWORD exit_code;
	BOOL success = false;

	TCHAR char_dll_path[(MAX_PATH + 1)] = { 0 };
	wcscpy_s(char_dll_path, dll_path.c_str());

	MODULEENTRY32 module_entry = { 0 };
	module_entry.dwSize = sizeof(MODULEENTRY32);
	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);
	Module32First(h_snapshot, &module_entry);
	do
	{
		if (!wcscmp(module_entry.szExePath, char_dll_path))
		{
			h_dll = module_entry.hModule;
			base_addr = module_entry.modBaseAddr;
		}
	}
	while (Module32Next(h_snapshot, &module_entry));
	CloseHandle(h_snapshot);

	if (h_dll && base_addr)
	{
		if ((h_process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, process_id)))
		{
			if ((h_kernel_32 = GetModuleHandle(L"kernel32.dll")))
			{
				if ((func_addr = GetProcAddress(h_kernel_32, "FreeLibrary")))
				{
					if ((h_thread = CreateRemoteThread(h_process, NULL, 0, (LPTHREAD_START_ROUTINE)func_addr, base_addr, 0, NULL)))
					{
						WaitForSingleObject(h_thread, INFINITE);
						if (GetExitCodeThread(h_thread, &exit_code))
						{
							success = (exit_code != 0) ? TRUE : FALSE;
						}
						CloseHandle(h_thread);
					}
				}
				//CloseHandle(h_kernel_32);
			}
			CloseHandle(h_process);
		}
	}

	return success;
}