#ifndef __MEM_BLOCK_H__
#define __MEM_BLOCK_H__

/** INCLUDES **/
#include <iostream>
#include <windows.h>
#include "Process.h"
#include "../../HallowLib/Type.h"

/** CLASSES **/

/* MemBlock */
class MemBlock
{
public:
	MemBlock(HANDLE process_handle, DWORD block_addr, int block_size, Type *block_type, void *block_data);
	~MemBlock();
	
	unsigned int get_block_size() const;
	Type* get_block_type() const;
	void* get_block_data() const;

	void read();
	void write(void *buffer);
	void debug();

private:
	HANDLE process_handle;
	DWORD block_addr;
	unsigned int block_size;
	Type *block_type;
	void *block_data;
};

#endif