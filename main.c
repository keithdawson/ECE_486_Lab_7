#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Global Vars
//Read = 1 Write = 0

unsigned int references, mainMemorySize, cacheSize, blockSize, setSize;
char replacementPolicy, fileName[25];
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
} cacheLineStruct;

void runStartUp();
cpuOutput * readFile(char * fileName);
void output(cpuOutput *, cacheLineStruct *, int, int, int, int, int);

void main() {
	int continu = 1;
	while (continu == 1) {
        runStartUp();
        cpuOutput *mmFile = readFile(fileName);
        //unsigned int operations = sizeof(mmFile) / sizeof(cpuOutput);             Probably don't need
        unsigned int addressLines = log2(mainMemorySize), offset = log2(blockSize), index = log2(
                (cacheSize / blockSize) / setSize);
		//printf("cacheSize = %d blockSize = %d setSize = %d index = %d\n", cacheSize, blockSize, setSize, index);
        unsigned int tagSize = addressLines - offset - index, cacheSections = cacheSize / blockSize;
        //cacheLineStruct* cacheLine = malloc(cacheSections * sizeof(cacheLineStruct*));
        cacheLineStruct cacheLine[cacheSections];
        //printf("Mark 3\n");
        for (unsigned int i = 0; i < cacheSections; i++) {
            //printf("Mark X; i = %d\n", i);
            cacheLine[i].blk = i;
            cacheLine[i].dirty = 0;
            cacheLine[i].valid = 0;
            cacheLine[i].tagSize = tagSize;
            cacheLine[i].tag = 0;
            cacheLine[i].lastReferencedLine = 0;
            cacheLine[i].firstReferencedLine = 0;
            strcpy(cacheLine[i].data, "");
        }
        //printf("Mark 4\n");
        int numSets = index * index;
        for (unsigned int i = 0; i < references; i++) {
            mmFile[i].mmblk = mmFile[i].address / blockSize;
            mmFile[i].cmset = mmFile[i].mmblk % numSets;
            mmFile[i].tag = mmFile[i].address >> (offset + index);
            mmFile[i].hitmiss = 0;
            int checks = 0;
            int cacheStartLine = mmFile[i].cmset * setSize;
            //printf("mmFile[i].readWrite = %d\nmm.address = %d\nmm.cmset = %d\nmm.tag = %d\n", mmFile[i].readWrite, mmFile[i].address, mmFile[i].cmset, mmFile[i].tag);
            if (mmFile[i].readWrite == 1) {
                while (setSize > checks) {
                    if ((cacheLine[cacheStartLine + checks].valid == 1) &&
                        (cacheLine[cacheStartLine + checks].tag == mmFile[i].tag)) {
                        mmFile[i].hitmiss = 1;
                        cacheLine[cacheStartLine + checks].lastReferencedLine = i;
                        break;
                    } else if (setSize == checks) {
                        for (unsigned int j = 0; j < checks; j++) {
                            if (cacheLine[cacheStartLine + j].valid == 0) {
                                cacheLine[cacheStartLine + j].valid = 1;
                                cacheLine[cacheStartLine + j].tag = mmFile[i].tag;
                                cacheLine[cacheStartLine + j].firstReferencedLine = i;
                                cacheLine[cacheStartLine + j].lastReferencedLine = i;
                                mmFile[i].hitmiss = 0;
                                break;
                            } else {
	                            //printf("Mark replace; i = %d\n", i);
                                if (replacementPolicy == 'L') {
                                    int LRU = 0;
                                    for (unsigned int k = 0; k < checks; k++) {
                                        if (cacheLine[cacheStartLine + k].lastReferencedLine > LRU) LRU = k;
                                    }
                                    cacheLine[cacheStartLine + LRU].tag = mmFile[i].tag;
                                    cacheLine[cacheStartLine + LRU].firstReferencedLine = i;
                                    cacheLine[cacheStartLine + LRU].lastReferencedLine = i;
                                    mmFile[i].hitmiss = 0;
                                } else {
                                    int FIFO = 99999999;
                                    for (unsigned int k = 0; k < checks; k++) {
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
	            //printf("Mark Write ; i = %d\nsetSize = %d\n", i, setSize);
	            checks = 0;
                while (setSize > checks) {
                    if ((cacheLine[cacheStartLine + checks].valid == 1) &&
                        (cacheLine[cacheStartLine + checks].tag == mmFile[i].tag)) {
                        mmFile[i].hitmiss = 1;
                        cacheLine[cacheStartLine + checks].dirty = 1;
                        cacheLine[cacheStartLine + checks].lastReferencedLine = i;
                        //printf("Mark write hit i = %d\n", i);
                        break;
                    } else if (setSize == checks) {
	                    //printf("Mark Write not valid; i = %d\n", i);
	                    for (unsigned int j = 0; j < checks; j++) {
                            if (cacheLine[cacheStartLine + j].valid == 0) {
                                cacheLine[cacheStartLine + j].valid = 1;
                                cacheLine[cacheStartLine + j].tag = mmFile[i].tag;
                                cacheLine[cacheStartLine + j].dirty = 1;
                                cacheLine[cacheStartLine + j].firstReferencedLine = i;
                                cacheLine[cacheStartLine + j].lastReferencedLine = i;
                                mmFile[i].hitmiss = 0;
                                break;
                            } else {
	                            //printf("Mark Write replace; i = %d\n", i);
	                            if (replacementPolicy == 'L') {
                                    int LRU = 0;
                                    for (unsigned int k = 0; k < checks; k++) {
                                        if (cacheLine[cacheStartLine + k].lastReferencedLine > LRU) LRU = k;
                                    }
                                    cacheLine[cacheStartLine + LRU].tag = mmFile[i].tag;
                                    cacheLine[cacheStartLine + LRU].dirty = 1;
                                    cacheLine[cacheStartLine + LRU].firstReferencedLine = i;
                                    cacheLine[cacheStartLine + LRU].lastReferencedLine = i;
                                    mmFile[i].hitmiss = 0;
                                } else {
                                    int FIFO = 99999999;
                                    for (unsigned int k = 0; k < checks; k++) {
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
                    checks++;
                }
            }
        }
        //printf("Mark End\n");
        output(mmFile, cacheLine, addressLines, offset, index, tagSize, cacheSections);
        printf("Continue? (y = yes, n = no); ");
        char *continueChar;
        scanf(" %c", continueChar);
        if (continueChar == "n") continu = 0;
    }
}

void output(cpuOutput * mmFile, cacheLineStruct * cacheLine, int addressLines, int offset, int index, int tagSize, int cacheSections){
    printf("\nSimulator Output:\nTotal address lines required = %d\n", addressLines);
    printf("Number of bits for offset = %d\n", offset);
    printf("Number of bits for index = %d\n", index);
    printf("Number of bits for tag = %d\n", tagSize);
    int totalCacheSize = cacheSize + cacheSections/8 * 2 + cacheSections/8 * tagSize;
    printf("Total cache size required = %d\n\n", totalCacheSize);
    printf("main memory address\t\tmm blk #\tcm set #\tcm blk #\thit/miss\n---------------------------------------------------------------------\n");
    for(int i=0;i < references;i++){
    	printf("\t\t%d\t\t\t\t\t%d\t\t\t%d\t\t%d", mmFile[i].address, mmFile[i].mmblk, mmFile[i].cmset, mmFile[i].cmset*setSize);
    	for(int j=1;j < setSize;j++) printf(" or %d", mmFile[i].cmset * setSize + j);
    	if (mmFile[i].hitmiss == 1) printf("\t\t\thit\n");
    	else printf("\t\t\tmiss\n");
    }
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