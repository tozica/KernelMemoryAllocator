#include"mySlab.h"
#include"buddy.h"
#include"Cache.h"
#include<stdio.h>

void* allocSlab(kmem_cache_t *cache)
{
	void* mem = alloc(sizeof(str_slab) + cache->blocksForSlubs * BLOCK_SIZE);
	str_slab* slab = (str_slab*)mem;
	slab->myMem = mem;
	slab->cashe = cache;
	slab->numbOfObjects = cache->numObjInSinglSlab;
	slab->numOfObjectsInUse = 0;
	slab->prevSlab = NULL;
	
	slab->memEnd = (char*)slab->myMem + sizeof(str_slab) + cache->blocksForSlubs * BLOCK_SIZE;

	str_slab* head = cache->halfEmpty;

	cache->halfEmpty = addElemToListCashe(cache->halfEmpty, slab);
	cache->numOfslabs++;

	slab->freeObjects = (char*)slab->myMem + sizeof(str_slab);
	void* location = slab->freeObjects;
	
	for (int i = 1; i < slab->cashe->numObjInSinglSlab - 1; i++) {
	
		*(int*)location = i;
		(char*)location += slab->cashe->sizeOfOneSlot;
	}

	*(int*)location = -1;
	
	return slab;
}

str_slab* clearSlab(str_slab * slab) {
	slab->freeObjects = (char*)slab->myMem + sizeof(str_slab);
	void* location = slab->freeObjects;
	slab->numOfObjectsInUse = 0;

	for (int i = 1; i < slab->cashe->numObjInSinglSlab - 1; i++) {

		*(int*)location = i;
		(char*)location += slab->cashe->sizeOfOneSlot;
	}

	*(int*)location = -1;
	return slab;
}

void writeSlab(str_slab *s)
{
	printf("numOfObjects %d\n", s->numbOfObjects);
	printf("numofObjInUse %d\n", s->numOfObjectsInUse);
	int i = 0;
	void* addres = s->freeObjects;
	while (*(int*)addres != -1){

		printf("%d\n", *(int*)addres);
		i++;
		addres = (char*)s->myMem + sizeof(str_slab) + (*(int*)addres * s->cashe->sizeOfOneSlot);
	}
}

void* allocSlabObject(str_slab* s) {
	
	void* ret = s->freeObjects;
	int index = *(int*)(s->freeObjects);
	if (index == -1)
		s->freeObjects = (char*)s->myMem + sizeof(str_slab);
	else
		s->freeObjects = (char*)s->myMem + sizeof(str_slab) + index* s->cashe->sizeOfOneSlot;
	s->numOfObjectsInUse++;
	index = *(int*)(s->freeObjects);
	if (index == -1 ) {
		s->cashe->halfEmpty = removeElemFromListCache(s->cashe->halfEmpty, s);
		s->cashe->full = addElemToListCashe(s->cashe->full, s);
	}

	*(int*)ret = -1;
	return ret;
	
}

str_slab* addElemToListCashe(str_slab *head, str_slab *elem) {
	if (head == NULL) {
		head = elem;
		head->prevSlab = NULL;
		head->nextSlab = NULL;
	}
	else {
		head->prevSlab = elem;
		elem->prevSlab = NULL;
		elem->nextSlab = head;
		head = elem;
	}
	return head;
}

str_slab* removeElemFromListCache(str_slab* head, str_slab* elem) {
	if (elem->prevSlab == NULL && elem->nextSlab == NULL) {
		head = NULL;
		return head;
	}

	if (elem->prevSlab == NULL && elem->nextSlab != NULL) {
		elem->nextSlab->prevSlab = NULL;
		head = elem->nextSlab;
		return head;
	}
	else if (elem->nextSlab == NULL && elem->prevSlab != NULL)
		elem->prevSlab->nextSlab = NULL;
	else {
		elem->prevSlab->nextSlab = elem->nextSlab;
		elem->nextSlab->prevSlab = elem->prevSlab;
	}

	elem->prevSlab = NULL;
	elem->nextSlab = NULL;

	return head;
}
