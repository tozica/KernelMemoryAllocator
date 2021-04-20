#include"slab.h"
#include"Cache.h"
#include "buddy.h"

extern str_buddy* globalBuddy;

void kmem_init(void* space, int block_num)
{
	skmem_init(space, block_num);
}

kmem_cache_t* kmem_cache_create(const char* name, size_t size, void(*ctor)(void*), void(*dtor)(void*))
{
	return skmem_cache_create(name, size, ctor, dtor);;
}

int kmem_cache_shrink(kmem_cache_t* cachep)
{
	int value = skmem_cache_shrink(cachep);
	return value;
}

void* kmem_cache_alloc(kmem_cache_t* cachep)
{
	void *value = skmem_cache_alloc(cachep);
	return value;
}

void kmem_cache_free(kmem_cache_t* cachep, void* objp)
{
	skmem_cache_free(cachep, objp);
}

void* kmalloc(size_t size)
{
	return skmalloc(size);
}

void kfree(const void* objp)
{
	skfree(objp);
}

void kmem_cache_destroy(kmem_cache_t* cachep)
{
	skmem_cache_destroy(cachep);
}

void kmem_cache_info(kmem_cache_t* cachep)
{
	skmem_cache_info(cachep);	
}

int kmem_cache_error(kmem_cache_t* cachep)
{
	int value =  skmem_cache_error(cachep);
	return value;
}
