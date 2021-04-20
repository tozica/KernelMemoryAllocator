#pragma once
#include<Windows.h>

#define NUMBuffer (17)


typedef struct str_buddy {
	int numOfBlocks, sizeOfArray;
	void* memory, *space;
	HANDLE mainMutex;
	struct kmem_cache_s* head;
	struct kmem_cache_s* bufferHead[NUMBuffer];
	
}str_buddy;

void buddy_init(void* space, int block_num);
int* doCast(str_buddy* buddy);
void* intToBlock(int i);
void freeBuddy();
void* alloc(unsigned int size);
void dealloc(void* point, unsigned int size);
void insertInGlobalBuddyArray(int blckNum, int blckCon);
void removeElemFromGlobalBuddy(int blockNum, int blckCon);
