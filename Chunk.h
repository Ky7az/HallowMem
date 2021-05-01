#ifndef __MEM_CHUNK_H__
#define __MEM_CHUNK_H__

/** INCLUDES **/
#include <iostream>
#include <windows.h>
#include "Process.h"

/** CLASSES **/

/* MemChunk */
class MemChunk
{
public:
	MemChunk(HANDLE process_handle, MEMORY_BASIC_INFORMATION *mbi);
	~MemChunk();

	DWORD get_chunk_addr() const;
	unsigned int get_chunk_size() const;
	unsigned char* get_chunk_buff() const;

	void read();
	void debug();

private:
	HANDLE process_handle;
	DWORD chunk_addr;
	unsigned int chunk_size;
	unsigned char *chunk_buff;
};

#endif
