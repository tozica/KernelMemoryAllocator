#include"buddy.h"
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<stdbool.h>
#define BLOCK_SIZE (4096)


str_buddy *globalBuddy = NULL;

void buddy_init(void* space, int block_num)
{
	if (space == NULL) {
		printf("Space is null! \n");
		exit(1);
	}

	
	globalBuddy = (str_buddy*)space;
	globalBuddy->head = NULL;

	int array = ceil(log2(block_num));
	globalBuddy->space = space;


	int budiesBlocks = (sizeof(str_buddy) + sizeof(int*) * array )/ BLOCK_SIZE;

	for (int i = 0; i < NUMBuffer; i++)
		globalBuddy->bufferHead[i] = NULL;


	if (budiesBlocks == 0) {
		budiesBlocks = 1;
		globalBuddy->numOfBlocks = block_num - 1;
	}
	else
		globalBuddy->numOfBlocks = block_num - budiesBlocks;


	globalBuddy->sizeOfArray = ceil(log2(globalBuddy->numOfBlocks));

	unsigned int tmpMemory = (unsigned int)((char*)space + budiesBlocks * BLOCK_SIZE);

	tmpMemory += BLOCK_SIZE - 1;
	size_t mask = BLOCK_SIZE - 1;
	tmpMemory &= ~mask;
	
	globalBuddy->mainMutex = CreateMutex(NULL, false, NULL);
	globalBuddy->memory = (void*)tmpMemory;

	for (int i = 0; i < globalBuddy->sizeOfArray; i++) {
		int* tmp = doCast(globalBuddy);
		tmp[i] = -1;
		//((int)((str_buddy*)globalBuddy + 1) + i) = -1'
	}

	doCast(globalBuddy)[globalBuddy->sizeOfArray] = 0;
	*(int*)(intToBlock(0)) = -1;

}

int* doCast(str_buddy* buddy) {
	return (int*)(buddy + 1);
}

void * intToBlock(int i) {
	return (char*)globalBuddy->memory + BLOCK_SIZE * i;
}
void freeBuddy() {

	for (int i = 0; i < globalBuddy->numOfBlocks; i++)
		*(int*)(intToBlock(i)) = -1;

	for (int i = 0; i < globalBuddy->sizeOfArray; i++) {
		int* tmp = doCast(globalBuddy);
		tmp[i] = -1;
	}


	CloseHandle(globalBuddy->mainMutex);

	globalBuddy->head = NULL;
	globalBuddy->memory = NULL;
	globalBuddy->space = NULL;
 }

void* alloc(unsigned int size)
{
	int blckNum = 0;
	void* ret = NULL;
	if (size % BLOCK_SIZE != 0)
		blckNum = size / BLOCK_SIZE + 1;
	else
		blckNum = size / BLOCK_SIZE;
	int nextPower = pow(2, ceil(log(blckNum) / log(2)));

	if (blckNum > globalBuddy->numOfBlocks) {
		return NULL;
	}

	if (doCast(globalBuddy)[(int)(log2(nextPower))] != -1) {
		int index = doCast(globalBuddy)[(int)(log2(nextPower))];
		int head = *((int*)intToBlock(index));
	    ret = intToBlock(index);

		doCast(globalBuddy)[(int)(log2(nextPower))] = head;
	}
	else {
		int step = 0;
		int i = 0;
		for (int i = (int)log2(nextPower); i <= globalBuddy->sizeOfArray; i++, step++) {
			int tmppp = doCast(globalBuddy)[i];
			if (doCast(globalBuddy)[i] != -1) {
				while (step > 0) {
					int tmp = doCast(globalBuddy)[i];
					int head = *((int*)intToBlock(tmp));
					doCast(globalBuddy)[i] = head;
					int size = (int)pow(2, i) / 2;
					
					i--;

					doCast(globalBuddy)[i] = tmp;
					*((int*)intToBlock(tmp)) = tmp + size;
					*((int*)intToBlock(tmp + size)) = -1;


					step--;
				}


				break;
			}
			
			
		}
		if (doCast(globalBuddy)[(int)(log2(nextPower))] != -1) {
			int index = doCast(globalBuddy)[(int)(log2(nextPower))];
			int head = *((int*)intToBlock(index));
			ret = intToBlock(index);
			

			doCast(globalBuddy)[(int)(log2(nextPower))] = head;
		}

	}

	return ret;
}

void insertInGlobalBuddyArray(int blckNum, int blckCon) {
	

	int head = doCast(globalBuddy)[(int)(log2(blckCon))];
	if (head == -1) {
		doCast(globalBuddy)[(int)(log2(blckCon))] = blckNum;
		*((int*)intToBlock(blckNum)) = -1;
	}
	else {
		int pret = -1;
		int tmp = blckNum;
		while (tmp > head) {
			pret = head;
			head = *((int*)(intToBlock(head)));
			if (head == -1)
				break;
		}
		if (head == -1)
			*(int*)(intToBlock(pret)) = blckNum;
		else if (pret == -1) {
			*((int*)(intToBlock(blckNum))) = head;
			doCast(globalBuddy)[(int)(log2(blckCon))] = blckNum;
		}
		else {
			*((int*)(intToBlock(pret))) = blckNum;
			*((int*)(intToBlock(blckNum))) = head;
		}
	}
}

void removeElemFromGlobalBuddy(int blockNum, int blckCon) {
	int head = doCast(globalBuddy)[(int)(log2(blckCon))];
	int pret = -1;
	
	while (head != blockNum) {
		pret = head;
		head = *(int*)intToBlock(head);
	}

	if (pret == -1) {
		doCast(globalBuddy)[(int)(log2(blckCon))] = *(int*)intToBlock(head);
		*(int*)intToBlock(head) = -1;
	}
	else {
		*(int*)intToBlock(pret) = *(int*)intToBlock(head);
		*(int*)intToBlock(head) = -1;
	}

}

void dealloc(void* point, unsigned int size)
{
	int blckCon =  ceil((double)size / BLOCK_SIZE);
	blckCon = pow(2, ceil(log(blckCon) / log(2)));

	int blckNum = ((char*)point - (char*)globalBuddy->memory) / BLOCK_SIZE;
	insertInGlobalBuddyArray(blckNum, blckCon);
	
	

	while (1) {
		
		int head = doCast(globalBuddy)[(int)(log2(blckCon))];
		int tmp = head;

		bool right = false;

		int condition = blckNum / blckCon;
		if (condition % 2 == 0)
			right = true;

		int pret = -1;
		while (tmp != blckNum) {
			pret = head;
			tmp = *(int*)intToBlock(tmp);
		}

		if (right) {
			int forward = *(int*)intToBlock(tmp);
			if (blckNum + blckCon == forward) {
				removeElemFromGlobalBuddy(blckNum, blckCon);
				removeElemFromGlobalBuddy(forward, blckCon);
				blckCon = blckCon * 2;
				insertInGlobalBuddyArray(blckNum, blckCon);
			}
			else
				break;
		}
		else {
			if (blckNum - blckCon == pret) {
				removeElemFromGlobalBuddy(blckNum, blckCon);
				removeElemFromGlobalBuddy(pret, blckCon);
				blckNum = pret;
				blckCon = blckCon * 2;
				insertInGlobalBuddyArray(pret, blckCon);
			}
			else
				break;
		}
	}
}


