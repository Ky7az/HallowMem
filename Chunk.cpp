#include "Chunk.h"

using namespace std;

MemChunk::MemChunk(HANDLE process_handle, MEMORY_BASIC_INFORMATION *mbi)
{
	this->process_handle = process_handle;
	this->chunk_addr = (DWORD)mbi->BaseAddress;
	this->chunk_size = mbi->RegionSize;
	this->chunk_buff = (unsigned char*)calloc(mbi->RegionSize, 1);
}

MemChunk::~MemChunk()
{
	free(chunk_buff);
}

DWORD MemChunk::get_chunk_addr() const { return chunk_addr; }
unsigned int MemChunk::get_chunk_size() const { return chunk_size; }
unsigned char* MemChunk::get_chunk_buff() const { return chunk_buff; }

void MemChunk::read()
{
	process_read(process_handle, chunk_addr, chunk_buff, chunk_size);
}

void MemChunk::debug()
{
	wcout << "0x" << hex << chunk_addr << dec;
	wcout << " - 0x" << hex << chunk_addr + chunk_size << dec;
	wcout << " <" << chunk_size << " bytes>" << endl;
}