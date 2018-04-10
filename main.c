#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Global Vars
//Read = 1 Write = 0

unsigned int references, mainMemorySize, cacheSize, blockSize, setSize;
char replacementPolicy, fileName[];
typedef struct cpu_Output{
	unsigned int readWrite;
	unsigned int address;
	unsigned int mmblk;
	unsigned int cmset;
	unsigned int cmblk;
	unsigned int hitmiss;
	unsigned int tag;
} cpuOutput;

typedef struct cache_Line{
	unsigned int blk;
	unsigned int dirty;
	unsigned int valid;
	unsigned int tagSize;
	unsigned int tag;
	unsigned int lastReferencedLine;
	unsigned int firstReferencedLine;
	char data[14];
} cacheLines;

void runStartUp();
cpuOutput * readFile(char * fileName);
void output(cpuOutput *, cacheLines *);

void main() {
	int continu = 1;
	while (continu == 1) {
		runStartUp();
		cpuOutput *mmFile = readFile(fileName);
		//unsigned int operations = sizeof(mmFile) / sizeof(cpuOutput);             Probably don't need
		unsigned int addressLines = log2(mainMemorySize), offset = log2(blockSize), tagSize = log2(
				(cacheSize / setSize) / blockSize);
		unsigned int index = addressLines - offset - tagSize;
		cacheLines *cacheLine;
		cacheLine = malloc(cacheSize * sizeof(cacheLines));
		for (unsigned int i = 0; i < cacheSize; i++) {
			cacheLine[i].blk = i;
			cacheLine[i].dirty = 0;
			cacheLine[i].valid = 0;
			cacheLine[i].tagSize = tagSize;
			cacheLine[i].tag = 0;
			cacheLine[i].lastReferencedLine = 0;
			cacheLine[i].firstReferencedLine = 0;
			strcpy(cacheLine[i].data, "");
		}
		int numSets = index * index;
		for (unsigned int i = 0; i < references; i++) {
			mmFile[i].mmblk = mmFile[i].address / blockSize;
			mmFile[i].cmset = mmFile[i].mmblk % numSets;
			mmFile[i].tag = mmFile[i].address >> (offset + index);
			mmFile[i].hitmiss = 0;
			int checks = 0;
			int cacheStartLine = mmFile[i].cmset * setSize;
			if (mmFile[i].readWrite == 1) {
				while (setSize > checks) {
					if ((cacheLine[cacheStartLine + checks].valid == 1) &&
						(cacheLine[cacheStartLine + checks].tag == mmFile[i].tag)) {
						mmFile[i].hitmiss = 1;
						cacheLine[cacheStartLine + checks].lastReferencedLine = i;
						break;
					}
					else if (setSize == checks){
						for (unsigned int j=0; j<checks; j++){
							if (cacheLine[cacheStartLine + j].valid == 0){
								cacheLine[cacheStartLine + j].valid = 1;
								cacheLine[cacheStartLine + j].tag = mmFile[i].tag;
								cacheLine[cacheStartLine + j].firstReferencedLine = i;
								cacheLine[cacheStartLine + j].lastReferencedLine = i;
								mmFile[i].hitmiss = 0;
								break;
							}
							else {
								if (replacementPolicy == 'L'){
									int LRU=0;
									for (unsigned int k=0; k<checks; k++){
										if (cacheLine[cacheStartLine + k].lastReferencedLine > LRU) LRU = k;
									}
									cacheLine[cacheStartLine + LRU].tag = mmFile[i].tag;
									cacheLine[cacheStartLine + LRU].firstReferencedLine = i;
									cacheLine[cacheStartLine + LRU].lastReferencedLine = i;
									mmFile[i].hitmiss = 0;
								}
								else {
									int FIFO=99999999;
									for (unsigned int k=0; k<checks; k++){
										if (cacheLine[cacheStartLine + k].firstReferencedLine < FIFO) FIFO = k;
									}
									cacheLine[cacheStartLine + FIFO].tag = mmFile[i].tag;
									cacheLine[cacheStartLine + FIFO].firstReferencedLine = i;
									cacheLine[cacheStartLine + FIFO].lastReferencedLine = i;
								}

							}
						}
					}
					checks++;
				}
			}
			else {
				checks = 0;
				while (setSize > checks) {
					if ((cacheLine[cacheStartLine + checks].valid == 1) &&
						(cacheLine[cacheStartLine + checks].tag == mmFile[i].tag)) {
						mmFile[i].hitmiss = 1;
						cacheLine[cacheStartLine + checks].dirty = 1;
						cacheLine[cacheStartLine + checks].lastReferencedLine = i;
						break;
					}
					else if (setSize == checks){
						for (unsigned int j=0; j<checks; j++){
							if (cacheLine[cacheStartLine + j].valid == 0){
								cacheLine[cacheStartLine + j].valid = 1;
								cacheLine[cacheStartLine + j].tag = mmFile[i].tag;
								cacheLine[cacheStartLine + j].dirty = 1;
								cacheLine[cacheStartLine + j].firstReferencedLine = i;
								cacheLine[cacheStartLine + j].lastReferencedLine = i;
								mmFile[i].hitmiss = 0;
								break;
							}
							else{
								if (replacementPolicy == 'L'){
									int LRU=0;
									for (unsigned int k=0; k<checks; k++){
										if (cacheLine[cacheStartLine + k].lastReferencedLine > LRU) LRU = k;
									}
									cacheLine[cacheStartLine + LRU].tag = mmFile[i].tag;
									cacheLine[cacheStartLine + LRU].dirty = 1;
									cacheLine[cacheStartLine + LRU].firstReferencedLine = i;
									cacheLine[cacheStartLine + LRU].lastReferencedLine = i;
									mmFile[i].hitmiss = 0;
								}
								else {
									int FIFO=99999999;
									for (unsigned int k=0; k<checks; k++){
										if (cacheLine[cacheStartLine + k].firstReferencedLine < FIFO) FIFO = k;
									}
									cacheLine[cacheStartLine + FIFO].tag = mmFile[i].tag;
									cacheLine[cacheStartLine + FIFO].dirty = 1;
									cacheLine[cacheStartLine + FIFO].firstReferencedLine = i;
									cacheLine[cacheStartLine + FIFO].lastReferencedLine = i;
								}
							}
                        }
					}
				}
			}
		}
		printf("Continue? (y = yes, n = no); ");
		char *continueChar;
		scanf(" %c", continueChar);
		if (continueChar == "n") continu = 0;
	}
    return;
}

void runStartUp(){
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

cpuOutput * readFile(char * fileName){
	FILE *fp = fopen(fileName, "r");
	char rW[1], addr[5], refTotal[12];
	fscanf(fp, "%s", refTotal);
	references = atoi(refTotal);
	cpuOutput * cpuFile;
	cpuFile = malloc (references * sizeof(cpuFile));
	int readArray[references][2];
	for (int i=0; i<references; i++){
		fscanf(fp, "%s", rW);
		if (rW == "R") cpuFile[i].readWrite = 1;
		else readArray[i][0] = 0;
		fscanf(fp, "%s", addr);
		cpuFile[i].address = atoi(addr);
	}
	return cpuFile;
}