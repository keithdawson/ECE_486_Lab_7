#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Global Vars for intake from startup function

unsigned int references, mainMemorySize, cacheSize, blockSize, setSize;
char replacementPolicy, fileName[25];
typedef struct cpu_Output{ //Struct for file and MM output
	unsigned int readWrite;
	unsigned int address;
	unsigned int mmblk;
	unsigned int cmset;
	unsigned int hitmiss;
	unsigned int tag;
} cpuOutput;

typedef struct cache_Line{ //Struct for cache lines
	unsigned int dirty;
	unsigned int valid;
	unsigned int tag;
	unsigned int lastReferencedLine;
	unsigned int firstReferencedLine;
	char data[14];
} cacheLineStruct;

//Functions that are used.

void runStartUp();
cpuOutput * readFile(char * fileName);
void output(cpuOutput *, cacheLineStruct *, int, int, int, int, int);

void main() {
	int continu = 1;
	while (continu == 1) {
		runStartUp();                             //Take input
		cpuOutput *mmFile = readFile(fileName);   //Read file
		//Calculate stats given input
		unsigned int addressLines = log2(mainMemorySize), offset = log2(blockSize), index = log2(
				(cacheSize / blockSize) / setSize);
		unsigned int tagSize = addressLines - offset - index, cacheSections = cacheSize / blockSize;
		cacheLineStruct cacheLine[cacheSections];
		for (unsigned int i = 0; i < cacheSections; i++) {		//Clear the cache
			cacheLine[i].dirty = 0;
			cacheLine[i].valid = 0;
			cacheLine[i].tag = 0;
			cacheLine[i].lastReferencedLine = 0;
			cacheLine[i].firstReferencedLine = 0;
			strcpy(cacheLine[i].data, "");
		}
		int numSets = cacheSections / setSize;
		for (unsigned int i = 0; i < references; i++) {		//Clear the MMFile
			mmFile[i].mmblk = mmFile[i].address / blockSize;
			mmFile[i].cmset = mmFile[i].mmblk % numSets;
			mmFile[i].tag = mmFile[i].address >> (offset + index);
			mmFile[i].hitmiss = 0;
		}
		for (unsigned int i = 0; i < references; i++) {		//Start going through the MMfile to drop into the cache
			int cacheStartLine = mmFile[i].cmset * setSize;
			if (mmFile[i].readWrite == 1) {
				for(int checks = 0;checks < setSize; checks++){		//Check within your set
					if ((cacheLine[cacheStartLine + checks].valid == 1) &&
						(cacheLine[cacheStartLine + checks].tag == mmFile[i].tag)) {		//Check for a valid and tag hit
						mmFile[i].hitmiss = 1;
						cacheLine[cacheStartLine + checks].lastReferencedLine = i;
						break;
					}
					if (setSize == checks+1) {
						for (unsigned int j = 0; j <= checks; j++) {
							if (cacheLine[cacheStartLine + j].valid == 0) {		//Input into first empty line
								cacheLine[cacheStartLine + j].valid = 1;
								cacheLine[cacheStartLine + j].tag = mmFile[i].tag;
								cacheLine[cacheStartLine + j].firstReferencedLine = i;
								cacheLine[cacheStartLine + j].lastReferencedLine = i;
								sprintf(cacheLine[cacheStartLine + j].data, "mm blk # %d", mmFile[i].mmblk);
								mmFile[i].hitmiss = 0;
								break;
							} else if (j==checks){
								if (replacementPolicy == 'L') {						//Implement each replacement policy
									int LRU = 0;
									for (unsigned int k = 0; k <= checks; k++) {	//Find the lowest lastReferencedLine
										if (cacheLine[cacheStartLine + k].lastReferencedLine < cacheLine[cacheStartLine + LRU].lastReferencedLine) LRU = k;
									}												//Put mmBlk there
									cacheLine[cacheStartLine + LRU].tag = mmFile[i].tag;
									cacheLine[cacheStartLine + LRU].firstReferencedLine = i;
									cacheLine[cacheStartLine + LRU].lastReferencedLine = i;
									cacheLine[cacheStartLine + LRU].dirty = 0;
									sprintf(cacheLine[cacheStartLine + LRU].data, "mm blk # %d", mmFile[i].mmblk);
									mmFile[i].hitmiss = 0;
								} else {
									int FIFO = 0;
									for (unsigned int k = 0; k <= checks; k++) {	//Find first referencedLine
										if (cacheLine[cacheStartLine + k].firstReferencedLine < cacheLine[cacheStartLine + FIFO].firstReferencedLine) FIFO = k;
                                    }												//Put mmBlk There
									cacheLine[cacheStartLine + FIFO].tag = mmFile[i].tag;
									cacheLine[cacheStartLine + FIFO].firstReferencedLine = i;
									cacheLine[cacheStartLine + FIFO].lastReferencedLine = i;
									cacheLine[cacheStartLine + FIFO].dirty = 0;
									sprintf(cacheLine[cacheStartLine + FIFO].data, "mm blk # %d", mmFile[i].mmblk);
									mmFile[i].hitmiss = 0;
								}

							}
						}
					}
				}
			}
			else {
				for(int checks = 0;checks < setSize; checks++){				//Write Hit check
					if ((cacheLine[cacheStartLine + checks].valid == 1) && (cacheLine[cacheStartLine + checks].tag == mmFile[i].tag)) {
						mmFile[i].hitmiss = 1;
						cacheLine[cacheStartLine + checks].dirty = 1;
						cacheLine[cacheStartLine + checks].lastReferencedLine = i;
						break;
					}
					if (setSize == checks+1) {
						for (unsigned int j = 0; j <= checks; j++) {			//Put mmblk into first empty spot
							if (cacheLine[cacheStartLine + j].valid == 0) {
								cacheLine[cacheStartLine + j].valid = 1;
								cacheLine[cacheStartLine + j].tag = mmFile[i].tag;
								cacheLine[cacheStartLine + j].dirty = 1;
								cacheLine[cacheStartLine + j].firstReferencedLine = i;
								cacheLine[cacheStartLine + j].lastReferencedLine = i;
								sprintf(cacheLine[cacheStartLine + j].data, "mm blk # %d", mmFile[i].mmblk);
								mmFile[i].hitmiss = 0;
								break;
							} else if (j==checks){
								if (replacementPolicy == 'L') {
									int LRU = 0;
									for (unsigned int k = 0; k <= checks; k++) {	//Find lowest lastReferencedLine
										if (cacheLine[cacheStartLine + k].lastReferencedLine < cacheLine[cacheStartLine + LRU].lastReferencedLine) LRU = k;
                                    }												//Put mmBlk there
									cacheLine[cacheStartLine + LRU].tag = mmFile[i].tag;
									cacheLine[cacheStartLine + LRU].dirty = 1;
									cacheLine[cacheStartLine + LRU].firstReferencedLine = i;
									cacheLine[cacheStartLine + LRU].lastReferencedLine = i;
									sprintf(cacheLine[cacheStartLine + LRU].data, "mm blk # %d", mmFile[i].mmblk);
									mmFile[i].hitmiss = 0;
								} else {
									int FIFO = 0;
									for (unsigned int k = 0; k <= checks; k++) {	//Find firstReferencedLine
										if (cacheLine[cacheStartLine + k].firstReferencedLine < cacheLine[cacheStartLine + FIFO].firstReferencedLine) FIFO = k;
									}												//Put mmBlk there
									cacheLine[cacheStartLine + FIFO].tag = mmFile[i].tag;
									cacheLine[cacheStartLine + FIFO].dirty = 1;
									cacheLine[cacheStartLine + FIFO].firstReferencedLine = i;
									cacheLine[cacheStartLine + FIFO].lastReferencedLine = i;
									sprintf(cacheLine[cacheStartLine + FIFO].data, "mm blk # %d", mmFile[i].mmblk);
									mmFile[i].hitmiss = 0;
								}
							}
						}
					}
				}
			}
		}

		output(mmFile, cacheLine, addressLines, offset, index, tagSize, cacheSections);	//Output and check if continue
		printf("\nContinue? (y = yes, n = no); ");
		char continueChar;
		scanf(" %c", &continueChar);
		if (continueChar == 'n') continu = 0;
	}
}

void output(cpuOutput * mmFile, cacheLineStruct * cacheLine, int addressLines, int offset, int index, int tagSize, int cacheSections){
	printf("\nSimulator Output:\nTotal address lines required = %d\n", addressLines);
	//Output end lines and calculate missing values
	printf("Number of bits for offset = %d\n", offset);
	printf("Number of bits for index = %d\n", index);
	printf("Number of bits for tag = %d\n", tagSize);
	int totalCacheSize = cacheSize + cacheSections/8 * 2 + cacheSections/8 * tagSize;
	printf("Total cache size required = %d\n\n", totalCacheSize);
	printf("main memory address\t\tmm blk #\tcm set #\tcm blk #\thit/miss\n---------------------------------------------------------------------\n");
	for(int i=0;i < references;i++){		//repeat and print each mmBlk line
		printf("\t\t%d\t\t\t\t\t%d\t\t\t%d\t\t %d", mmFile[i].address, mmFile[i].mmblk, mmFile[i].cmset, mmFile[i].cmset*setSize);
		if (setSize == 2) printf(" or %d", mmFile[i].cmset * setSize + 1);
		if (setSize > 2) printf(" to %d", mmFile[i].cmset * setSize + setSize - 1);
		if (mmFile[i].hitmiss == 1) printf("\t\t\thit\n");
		else printf("\t\t\tmiss\n");
	}
	//Calculate hit rate
	int hitsPoss=0, hits=0, hitArray[mainMemorySize/blockSize];
	for(int i=0;i < mainMemorySize/blockSize;i++) hitArray[i]=0;
	for(int i=0;i < references;i++) {
		hitArray[mmFile[i].mmblk]++;
		if (mmFile[i].hitmiss == 1) hits++;
	}
	//Print cache output
	for(int i=0;i < mainMemorySize/blockSize;i++)	if (hitArray[i] > 1) hitsPoss = hitsPoss + (hitArray[i]-1);
	float hitsF = hits, hitsPossF = hitsPoss, referencesF = references;
	printf("\n\nHighest possible hit rate = %d/%d = %2.0f%%\n", hitsPoss, references, hitsPossF/referencesF*100.0);
	printf("Actual hit rate = %d/%d = %2.0f%%\n\n", hits, references, hitsF/referencesF*100.0);
	printf("Final \"status\" of the cache:\nCache blk #\t\tdirty bit\t\tvalid bit\ttag\t\t\tData\n");
	printf("--------------------------------------------------------------\n");
	for(int i=0;i < cacheSize/blockSize;i++){
		printf("\t%d\t\t\t\t%d\t\t\t\t%d\t\t", i, cacheLine[i].dirty, cacheLine[i].valid);
		if (cacheLine[i].valid == 0){ 		//Print correct number of xes
			for(int k=0;k<tagSize;k++) printf("x");
			if (tagSize < 4) printf("\t\t\t");
			else printf("\t\t");
			for(int k=0;k<tagSize;k++) printf("x");
			printf("\n");
		}
		else{
			//Convert int tag to printable binary chars
			char tagOutput[8];
			unsigned int tag = cacheLine[i].tag;
			for(int jk=0;jk<8;jk++) tagOutput[jk] = '\0';
			for(int l=0;l<tagSize;l++){
				if (tag % 2 == 0) tagOutput[tagSize - l - 1] = '0';
				else tagOutput[tagSize - l - 1] = '1';
				tag = tag / 2;
			}
			printf("%s\t\t%s\n",tagOutput ,cacheLine[i].data);
		}
	}
}


void runStartUp(){ //Print startup
	printf("Enter the size of main memory in bytes: ");
	scanf(" %d", &mainMemorySize);
	printf("Enter the size of the cache in bytes: ");
	scanf(" %d", &cacheSize);
	printf("Enter the cache line/block size: ");
	scanf(" %d", &blockSize);
	printf("Enter the degree of set-associativity (input n for an n-way set-associative mapping): ");
	scanf(" %d", &setSize);
	printf("Enter the replacement policy (L = LRU, F = FIFO): ");
	scanf(" %c", &replacementPolicy);
	printf("Enter the name of the input file containing the list of memory references generated by the CPU: ");
	scanf(" %s", fileName);
}

cpuOutput * readFile(char * fileName){ //Take input file and put it into mmBlk Struct
	FILE *fp = fopen(fileName, "r");
	char rW;
	unsigned int addr;
	fscanf(fp, "%d", &references);
	cpuOutput * cpuFile;
	cpuFile = malloc (references * sizeof(cpuFile));
	int readArray[references][2];
	for (int i=0; i<references; i++){
		fscanf(fp, " %c", &rW);
		if (rW == 'R') cpuFile[i].readWrite = 1;
		else readArray[i][0] = 0;
		fscanf(fp, " %d", &addr);
		cpuFile[i].address = addr;
	}
	return cpuFile;
}