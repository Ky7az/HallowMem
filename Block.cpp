#include "Block.h"
using namespace std;

MemBlock::MemBlock(HANDLE process_handle, DWORD block_addr, int block_size, Type *block_type, void *block_data)
{
	this->process_handle = process_handle;
	this->block_addr = block_addr;
	this->block_size = block_size;
	this->block_type = block_type;
	this->block_data = block_data;
}

MemBlock::~MemBlock()
{
}

unsigned int MemBlock::get_block_size() const { return block_size; }
Type* MemBlock::get_block_type() const { return block_type; }
void* MemBlock::get_block_data() const { return block_data; }

void MemBlock::read()
{
	process_read(process_handle, block_addr, block_data, block_size);
}

void MemBlock::write(void *buffer)
{
	process_write(process_handle, block_addr, buffer, sizeof(buffer));
}

void MemBlock::debug()
{
	wcout << "0x" << hex << block_addr << dec;
	wcout << " <";
	block_type->print(block_data);
	wcout << ">" << endl;
}