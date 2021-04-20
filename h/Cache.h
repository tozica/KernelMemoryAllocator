#pragma once
#include<stdbool.h>
#include<Windows.h>
typedef void (*ctor)(void*);
typedef void (*dtor)(void*);

typedef struct kmem_cache_s {
	unsigned int size;
	struct str_slab* empty, * halfEmpty, * full;
	struct kmem_cache_s* nextCashe, *prevCashe;
    bool shrink;

	HANDLE mutex;
	const char* name;
	unsigned int sizeOfOneSlot;
	int sizeInBlocks, numOfslabs, numObjInSinglSlab, blocksForSlubs, err;
	void* myMem;

	ctor constructor;
	dtor destructor;

}kmem_cache_t;
void skmem_init(void* space, int block_num);

kmem_cache_t* skmem_cache_create(const char* name, size_t size,
	void (*ctor)(void*),
	void (*dtor)(void*)); // Allocate cache
int skmem_cache_shrink(kmem_cache_t* cachep); // Shrink cache
void* skmem_cache_alloc(kmem_cache_t* cachep); // Allocate one object from cache
void skmem_cache_free(kmem_cache_t* cachep, void* objp); // Deallocate one object from cache
void* skmalloc(size_t size); // Alloacate one small memory buffer
void skfree(const void* objp); // Deallocate one small memory buffer
void skmem_cache_destroy(kmem_cache_t* cachep); // Deallocate cache
void skmem_cache_info(kmem_cache_t* cachep); // Print cache info
int skmem_cache_error(kmem_cache_t* cachep); // Print error message
int calculateObjectsInSlab(unsigned int size, int*);
writeCache(kmem_cache_t* cache);
kmem_cache_t* removeCacheFromBuddyList(kmem_cache_t* head, kmem_cache_t* elem);