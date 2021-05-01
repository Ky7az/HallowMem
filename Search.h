#ifndef __MEM_SEARCH_H__
#define __MEM_SEARCH_H__

/** INCLUDES **/
#include <iostream>
#include <string>
#include "..\..\HallowLib\Queue.h"
#include "..\..\HallowLib\Type.h"
#include "Process.h"
#include "Block.h"

/** DEFINES **/
typedef enum
{
	MS_COND_EQUALS,
	MS_COND_CHANGED,
	MS_COND_UNCHANGED,
	MS_COND_INCREASED,
	MS_COND_DECREASED,

} MemSearchCond;

/** CLASSES **/

/* MemSearch */
class MemSearch
{
public:
	MemSearch(MemProc *mp, TypeEnum mst, std::string search);
	MemSearch(MemProc *mp, unsigned char *pattern, std::string mask);
	~MemSearch();

	Queue* get_mem_blocks() const;

	bool init(std::string search);
	void start();
	void update(MemSearchCond msc, std::string search);
	void refresh();
	void res();
	void debug();

private:
	MemProc *mp;
	TypeEnum mst;
	MemSearchCond msc;
	unsigned int size;
	std::string mask;
	void *search;
	unsigned char *pattern;
	Queue *mem_blocks;
};

/** FUNCTIONS **/
int find_pattern(unsigned char *buffer, unsigned int size, unsigned char *pattern, char *mask);
int match_pattern(unsigned char *buffer, unsigned char *pattern, char *mask);

#endif
