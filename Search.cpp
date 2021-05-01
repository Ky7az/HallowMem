#include "Search.h"
using namespace std;

/* MemSearch */
MemSearch::MemSearch(MemProc *mp, TypeEnum mst, string search)
{
	if (search.empty())
		throw exception();

	this->mp = mp;
	this->mst = mst;
	this->msc = MS_COND_EQUALS;
	this->mem_blocks = new Queue();

	init(search);
	start();
}

MemSearch::MemSearch(MemProc *mp, unsigned char *pattern, string mask)
{
	this->mp = mp;
	this->mst = TYPE_BYTES;
	this->msc = MS_COND_EQUALS;
	this->size = mask.length();
	this->mask = mask;
	this->pattern = pattern;
	this->mem_blocks = new Queue();

	start();
}

MemSearch::~MemSearch()
{
	delete search;
	delete pattern;

	if (mem_blocks != NULL) {
		//Queue_free(ms->mem_blocks, MemBlock_free);
		delete mem_blocks;
	}
}

Queue* MemSearch::get_mem_blocks() const { return mem_blocks; }

bool MemSearch::init(string search)
{
	if (search.empty())
		return false;

	switch (mst) {
		case TYPE_BYTE:
		{
			int s_int = strtol(search.c_str(), NULL, 10);
			// Size
			this->size = sizeof(byte);
			// Search
			this->search = new byte(s_int);
			break;
		}
		case TYPE_SHORT:
		{
			int s_int = strtol(search.c_str(), NULL, 10);
			// Size
			this->size = sizeof(short);
			// Search
			this->search = new short(s_int);
			break;
		}
		case TYPE_INTEGER:
		{
			int s_int = strtol(search.c_str(), NULL, 10);
			// Size
			this->size = sizeof(int);
			// Search
			this->search = new int(s_int);
			break;
		}
		case TYPE_FLOAT:
		{
			float s_float = strtof(search.c_str(), NULL);
			// Size
			this->size = sizeof(float);
			// Search
			this->search = new float(s_float);
			break;
		}
		case TYPE_DOUBLE:
		{
			double s_double = strtod(search.c_str(), NULL);
			// Size
			this->size = sizeof(double);
			// Search
			this->search = new double(s_double);
			break;
		}
		case TYPE_STRING:
		{
			// Size
			this->size = search.length();
			// Search
			this->search = (char*)search.c_str();
			break;
		}
	}

	// Mask
	this->mask = string(size, 'x');
	// Pattern
	this->pattern = new unsigned char[size];
	memcpy(pattern, this->search, size);

	return true;
}

void MemSearch::start()
{
	int offset;
	int total;

	if (mp->get_mem_chunks() == NULL)
		return;

	Node *n = mp->get_mem_chunks()->get_head();
	while (n != NULL) {

		offset = 0;
		total = 0;
		MemChunk *mc = NULL;
		mc = (MemChunk*)n->get_data();

		do {
			if ((offset = find_pattern(mc->get_chunk_buff() + total, mc->get_chunk_size() - total, pattern, (char*)mask.c_str())) != -1) {

				Type *block_type = init_type_template(mst);
				MemBlock *mb = new MemBlock(mp->get_process_handle(), mc->get_chunk_addr() + total + offset, size, block_type, &mc->get_chunk_buff()[total + offset]);
				mem_blocks->add(mb);
				total += offset + size;
			}
		} while (offset != -1);

		n = n->get_next();
	}
}

void MemSearch::update(MemSearchCond msc, string search)
{
	void *before_val;
	void *after_val;
	bool is_match = false;

	if (mem_blocks == NULL)
		return;

	if (msc == MS_COND_EQUALS)
		if (!init(search))
			return;

	Node *n = mem_blocks->get_head();
	while (n != NULL) {

		MemBlock *mb = (MemBlock*)n->get_data();
		Type *block_type = mb->get_block_type();

		before_val = block_type->allocate();
		block_type->copy(before_val, mb->get_block_data());

		mb->read();
		after_val = mb->get_block_data();

		switch (msc) {
			case MS_COND_EQUALS:
				is_match = match_pattern((unsigned char*)after_val, pattern, const_cast<char*>(mask.c_str()));
				break;
			case MS_COND_CHANGED:
				is_match = block_type->is_different(after_val, before_val);
				break;
			case MS_COND_UNCHANGED:
				is_match = block_type->is_equal(after_val, before_val);
				break;
			case MS_COND_INCREASED:
				is_match = block_type->is_greater(after_val, before_val);
				break;
			case MS_COND_DECREASED:
				is_match = block_type->is_lower(after_val, before_val);
				break;
		}

		Node *n_next = n->get_next();
		delete before_val;

		if (!is_match) {
			mem_blocks->rem(n);
		}

		n = n_next;
	}
}

void MemSearch::refresh()
{
	if (mem_blocks == NULL)
		return;

	Node *n = mem_blocks->get_head();
	while (n != NULL) {
		MemBlock *mb = (MemBlock*)n->get_data();
		mb->read();
		n = n->get_next();
	}
}

void MemSearch::res()
{
	if (mem_blocks == NULL)
		return;

	Node *n = mem_blocks->get_head();
	while (n != NULL) {
		MemBlock *mb = (MemBlock*)n->get_data();
		mb->debug();
		n = n->get_next();
	}
	cout << mem_blocks->get_len() << " matches !" << endl;
}

void MemSearch::debug()
{
	wcout << "Search Type = " << mst << "\n";
	wcout << "Search Cond = " << msc << "\n";
}


int find_pattern(unsigned char *buffer, unsigned int size, unsigned char *pattern, char *mask)
{
	for (unsigned int i = 0; i < size; i++) {
		if (match_pattern(buffer + i, pattern, mask)) {
			return i;
		}
	}
	return -1;
}

int match_pattern(unsigned char *buffer, unsigned char *pattern, char *mask)
{
	for (; *mask; ++mask, ++buffer, ++pattern) {
		if (*mask == 'x' && *buffer != *pattern)
			return 0;
	}
	return (*mask == 0);
}