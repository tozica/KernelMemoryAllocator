#pragma once
#include"slab.h"

typedef struct str_slab {
	kmem_cache_t* cashe;
	struct str_slab* nextSlab, *prevSlab;
	unsigned numbOfObjects, numOfObjectsInUse, blocks;
	void* myMem, *memEnd;
	void* freeObjects;

}str_slab;

void* allocSlab(kmem_cache_t *cache);
void writeSlab(str_slab* s);
void* allocSlabObject(str_slab* );
str_slab* addElemToListCashe(str_slab*, str_slab* elem);
str_slab* removeElemFromListCache(str_slab* head, str_slab* elem);
str_slab* clearSlab(str_slab* slab);