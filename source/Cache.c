#include"Cache.h"
#include"slab.h"
#include<stdbool.h>
#include"buddy.h"
#include"mySlab.h"
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<math.h>


extern str_buddy* globalBuddy;

int calculateObjectsInSlab(unsigned int size, int *i) {
	int blocksInSlab = 0;
	unsigned usableSpace = BLOCK_SIZE - sizeof(str_slab);
	if (usableSpace < size)
		while (usableSpace < size) {
			usableSpace += BLOCK_SIZE;
			(*i)++;
		}

	return usableSpace/size;
	
}

writeCache(kmem_cache_t* cache)
{
	
	printf("Name %s\n", cache->name);
	
	printf("empty slabs \n");
	str_slab* tmpHead = cache->empty;
	while (tmpHead) {
		writeSlab(tmpHead);
		tmpHead = tmpHead->nextSlab;
		
	}
	printf("half empty slabs \n");
	 tmpHead = cache->halfEmpty;
	while (tmpHead) {
		writeSlab(tmpHead);
		tmpHead = tmpHead->nextSlab;
	}

	printf("full slabs \n");
	 tmpHead = cache->full;
	while (tmpHead) {
		writeSlab(tmpHead);
		tmpHead = tmpHead->nextSlab;
	}
}

kmem_cache_t* alloc_single_cache(const char* name, size_t size, void (*ctor)(void*), void (*dtor)(void*)) {
	
	void* tmp = alloc(sizeof(kmem_cache_t));

	if (!tmp)
		return NULL;

	kmem_cache_t* cache = (kmem_cache_t*)tmp;
	cache->full = cache->empty = cache->halfEmpty = NULL;
	cache->constructor = ctor;
	cache->destructor = dtor;
	cache->name = name;
	cache->myMem = tmp;
	cache->sizeOfOneSlot = size;
	int index = 0;
	cache->numObjInSinglSlab = calculateObjectsInSlab(size, &index);
	cache->nextCashe = NULL;
	cache->numOfslabs = 0;
	cache->shrink = true;
	cache->sizeInBlocks = ceil((double)sizeof(kmem_cache_t) / BLOCK_SIZE);
	cache->err = 0;
	cache->mutex = CreateMutex(NULL, false, NULL);
	if (index > 0)
		cache->blocksForSlubs = index + 1;
	else
		cache->blocksForSlubs = 1;

	return cache;
}

kmem_cache_t* skmem_cache_create(const char* name, size_t size, void (*ctor)(void*), void (*dtor)(void*)) {
	bool flag = false;
	kmem_cache_t* ret;
	if (size < sizeof(int))
		size = sizeof(int);
	kmem_cache_t* iter = globalBuddy->head;
	while (iter != NULL) {
		if (iter->sizeOfOneSlot == size) {
			flag = true;
			break;
		}
		iter = iter->nextCashe;
	}

	if (!flag) {
		kmem_cache_t* cache = alloc_single_cache(name, size, ctor, dtor);
		if (cache == NULL) {
			printf("No enoguh space to alocate cache!");
			return NULL;
		}

		if (globalBuddy->head == NULL) {
			cache->prevCashe = NULL;
			cache->nextCashe = NULL;
			globalBuddy->head = cache;
			ret = globalBuddy->head;

		}
		else {
			cache->nextCashe = globalBuddy->head;
			globalBuddy->head->prevCashe = cache;
			globalBuddy->head = cache;
			cache->prevCashe = NULL;
		}

		ret = cache;
	}
	else
		ret = iter;

	
	return ret;
}

void* skmem_cache_alloc(kmem_cache_t* cachep) {
	str_slab* tmp = cachep->halfEmpty;
	if (!cachep->halfEmpty && !cachep->empty) {
		tmp = allocSlab(cachep);
		if (tmp == NULL) {
			//printf("No enough memory to allocate one object");
			cachep->err = 1;
			
			return NULL;
		}
		cachep->shrink = false;
	}
	else if (cachep->empty && !cachep->halfEmpty) {
		tmp = cachep->empty;
		cachep->empty = removeElemFromListCache(cachep->empty, cachep->empty);
		cachep->halfEmpty = addElemToListCashe(cachep->halfEmpty, tmp);
	}
	
	void* ret = allocSlabObject(tmp);
	if(cachep->constructor != NULL)
		cachep->constructor(ret);
	
	return ret;

}

void skmem_init(void* space, int block_num) {
	buddy_init(space, block_num);
}

void skmem_cache_free(kmem_cache_t* cachep, void* objp) {
	str_slab* iter = cachep->halfEmpty;
	str_slab* find = NULL;
	while (iter) {
		if (iter->myMem < objp && iter->memEnd > objp) {
			find = iter;
			break;
		}
		iter = iter->nextSlab;
	}

	if (find) {
		int index;
		find->numOfObjectsInUse* find->cashe->sizeOfOneSlot;
		if (find->numOfObjectsInUse == find->numbOfObjects - 2)
			index = -1;
		else
		    index = ((char*)find->freeObjects - ((char*)cachep->halfEmpty->myMem + sizeof(str_slab))) / cachep->sizeOfOneSlot;
		find->freeObjects = objp;
		*((int*)find->freeObjects) = index;
		find->numOfObjectsInUse--;
		
		if (find->numOfObjectsInUse == 0) {
			cachep->halfEmpty = removeElemFromListCache(cachep->halfEmpty, find);
			find = clearSlab(find);
			cachep->empty = addElemToListCashe(cachep->empty, find);
		}
		if (cachep->destructor)
			cachep->destructor(objp);
	
		return;
	}

	find = NULL;
	iter = cachep->full;
	while (iter) {
		if (iter->myMem < objp && iter->memEnd > objp) {
			find = iter;
			break;
		}
		iter = iter->nextSlab;
	}

	if (find) {
		int index;
		find->numOfObjectsInUse* find->cashe->sizeOfOneSlot;
		if (find->numOfObjectsInUse == find->numbOfObjects - 2)
			index = -1;
		else
			index = ((char*)find->freeObjects - ((char*)cachep->full ->myMem + sizeof(str_slab))) / cachep->sizeOfOneSlot;
		find->freeObjects = objp;
		*((int*)find->freeObjects) = index;
		find->numOfObjectsInUse--;

		if (cachep->destructor)
			cachep->destructor(objp);
		cachep->full = removeElemFromListCache(cachep->full, find);
		cachep->halfEmpty = addElemToListCashe(cachep->halfEmpty, find);
		

	}
	if (cachep->empty != NULL) {
		find = cachep->empty;
		while (find != NULL) {
			clearSlab(find);
			find = find->nextSlab;
		}
	}


}

int skmem_cache_shrink(kmem_cache_t* cachep) {
	int ret = 0;
	static bool firstShrink = false;
	if (cachep->shrink || !firstShrink) {
		str_slab* head = cachep->empty;
		
		while (head != NULL) {
			head = cachep->empty->nextSlab;
			dealloc(cachep->empty->myMem, (char*)cachep->empty->memEnd - (char*)cachep->empty->myMem);
			ret = ret + ((char*)cachep->empty->memEnd - (char*)cachep->empty->myMem) / BLOCK_SIZE;
			cachep->empty = head;
			cachep->numOfslabs--;
		}
	}
	else {
		cachep->shrink = true;
	}

	firstShrink = true;
	
	return ret;
}

void* skmalloc(size_t size) {
	char name[20];
	kmem_cache_t* cache;
	void* retMem = NULL;
    size = (int)ceil(log2(size));
	sprintf_s(name, sizeof(name), "size-%d", size);
	

	
	if (size < 5 && size > 17) {
		return NULL;
	}


	if (globalBuddy->bufferHead[size] != NULL) {
		cache = globalBuddy->bufferHead[size];
		retMem = skmem_cache_alloc(cache);
		
	}
	else {
		cache = alloc_single_cache(name, pow(2, size), NULL, NULL);

		if (!cache) {
			printf("No enoguh memory for cache");
			return NULL;
		}
		globalBuddy->bufferHead[size] = cache;
		retMem = kmem_cache_alloc(cache);
		
	}
	return retMem;

}
void skfree(const void* objp) {
	kmem_cache_t* cache = NULL;
	for (int i = 0; i < NUMBuffer; i++)
		if (globalBuddy->bufferHead[i] != NULL) {
			
			kmem_cache_free(globalBuddy->bufferHead[i], objp);
			
		}
}

kmem_cache_t * removeCacheFromBuddyList(kmem_cache_t* head, kmem_cache_t* elem) {
	if (elem->prevCashe == NULL && elem->nextCashe == NULL) {
		head = NULL;
		return head;
	}

	if (elem->prevCashe == NULL && elem->nextCashe != NULL) {
		elem->nextCashe->prevCashe = NULL;
		head = elem->nextCashe;
		return head;
	}
	else if (elem->nextCashe == NULL && elem->prevCashe != NULL)
		elem->prevCashe->nextCashe = NULL;
	else {
		elem->prevCashe->nextCashe = elem->nextCashe;
		elem->nextCashe->prevCashe = elem->prevCashe;
	}

	elem->prevCashe = NULL;
	elem->nextCashe = NULL;
	
	return head;
}

int skmem_cache_error(kmem_cache_t* cachep) {
	switch (cachep->err)
	{
	case 0:
		printf("Errors don't exist\n\n");
		break;
	case 1:
		printf("No enough space for allocation\n\n");
		break;
	case 2:
		printf("In cache stil exist object, before deleting cache you must delete all objects\n\n");
		break;
	default:
		printf("Unknown error ocured!\n\n");
		break;
	}
	
	return cachep->err;
}

void skmem_cache_destroy(kmem_cache_t* cachep) {
		
	str_slab* temp = cachep->empty;

	while (cachep->empty != NULL) {
		cachep->empty = cachep->empty->nextSlab;
		dealloc(temp->myMem, (char*)temp->memEnd - (char*)temp->myMem);
		temp = cachep->empty;
	}

	if (cachep->halfEmpty || cachep->full) {
		cachep->err = 2;
		return;
	}

	cachep->constructor = NULL;
	cachep->destructor = NULL;

	globalBuddy->head = removeCacheFromBuddyList(globalBuddy->head, cachep);

	dealloc(cachep, cachep->sizeInBlocks * BLOCK_SIZE);
}

void skmem_cache_info(kmem_cache_t* cachep) {
	printf("Information about cache!\n\n");
	printf("Name of the cache %s\n", cachep->name);
	printf("Size of one object in bytes %u\n", cachep->sizeOfOneSlot);
	printf("Size of whole cache in blocks %d\n", cachep->blocksForSlubs);
	printf("Number of slabs in cache %d\n", cachep->numOfslabs);
	printf("Number of obj in one slub %d\n", cachep->numObjInSinglSlab);

	int usin = 0, count = 0;
	str_slab* iter = cachep->halfEmpty;
	
	while (iter) {
		count = count + iter->numbOfObjects;
		usin = usin + iter->numOfObjectsInUse;
		iter = iter->nextSlab;
	}

	iter = cachep->full;
	while (iter) {
		count = count + iter->numbOfObjects;
		usin = usin + iter->numOfObjectsInUse;
		iter = iter->nextSlab;
	}

	iter = cachep->empty;
	while (iter) {
		count = count + iter->numbOfObjects;
		usin = usin + iter->numOfObjectsInUse;
		iter = iter->nextSlab;
	}

	double fulfil = (double)usin / (double)count * 100;

	printf("Occupancy rate %f\n\n", fulfil);

}
