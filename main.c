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
		}
        for (unsigned int i = 0; i < references; i++) {
            int cacheStartLine = mmFile[i].cmset * setSize;
            printf("NEXT LINE\nmmFile[i].readWrite = %d\nmm.address = %d\nmm.blk = %d\nmm.cmset = %d\nmm.tag = %d\nsetSize = %d\ncacheStartLine = %d\n", mmFile[i].readWrite, mmFile[i].address,mmFile[i].mmblk, mmFile[i].cmset, mmFile[i].tag, setSize, cacheStartLine);
            if (mmFile[i].readWrite == 1) {
				for(int checks = 0;checks < setSize; checks++){
					printf("cacheLine[%d].valid = %d; cacheLine[%d].tag = %d", cacheStartLine+checks, cacheLine[cacheStartLine + checks].valid, cacheStartLine+checks, cacheLine[cacheStartLine + checks].tag);
                    if ((cacheLine[cacheStartLine + checks].valid == 1) &&
                        (cacheLine[cacheStartLine + checks].tag == mmFile[i].tag)) {
                    	printf("Hit cache %d\n", cacheStartLine+checks);
                        mmFile[i].hitmiss = 1;
                        cacheLine[cacheStartLine + checks].lastReferencedLine = i;
                        break;
                    }
                    printf("setSize = %d and checks = %d\n", setSize, checks);
                    if (setSize == checks+1) {
                    	printf("Mark NOW\n");
                        for (unsigned int j = 0; j < checks+1; j++) {
                            if (cacheLine[cacheStartLine + j].valid == 0) {
                            	printf("Valid = 0 Miss cmblk %d\n", cacheStartLine+j);
                                cacheLine[cacheStartLine + j].valid = 1;
                                cacheLine[cacheStartLine + j].tag = mmFile[i].tag;
                                cacheLine[cacheStartLine + j].firstReferencedLine = i;
                                cacheLine[cacheStartLine + j].lastReferencedLine = i;
	                            sprintf(cacheLine[cacheStartLine + j].data, "mm blk # %d", mmFile[i].mmblk);
                                mmFile[i].hitmiss = 0;
                                break;
                            } else if (j==checks){
	                            printf("Mark replace; i = %d, cache = %d\n", i, cacheStartLine+checks);
                                if (replacementPolicy == 'L') {
                                    int LRU = 0;
                                    for (unsigned int k = 0; k < checks; k++) {
                                        if (cacheLine[cacheStartLine + k].lastReferencedLine > LRU) LRU = k;
                                    }
                                    cacheLine[cacheStartLine + LRU].tag = mmFile[i].tag;
                                    cacheLine[cacheStartLine + LRU].firstReferencedLine = i;
                                    cacheLine[cacheStartLine + LRU].lastReferencedLine = i;
	                                cacheLine[cacheStartLine + LRU].dirty = 0;
	                                sprintf(cacheLine[cacheStartLine + LRU].data, "mm blk # %d", mmFile[i].mmblk);
                                    mmFile[i].hitmiss = 0;
                                } else {
                                    int FIFO = 99999999;
                                    for (unsigned int k = 0; k < checks; k++) {
                                        if (cacheLine[cacheStartLine + k].firstReferencedLine < FIFO) FIFO = k;
                                    }
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
	            printf("Mark Write; i = %d\nsetSize = %d\n", i, setSize);
				for(int checks = 0;checks < setSize; checks++){
					printf("cacheLine[%d].valid = %d; cacheLine[%d].tag = %d\n", cacheStartLine+checks, cacheLine[cacheStartLine + checks].valid, cacheStartLine+checks, cacheLine[cacheStartLine + checks].tag);
					if ((cacheLine[cacheStartLine + checks].valid == 1) && (cacheLine[cacheStartLine + checks].tag == mmFile[i].tag)) {
                        mmFile[i].hitmiss = 1;
                        cacheLine[cacheStartLine + checks].dirty = 1;
                        cacheLine[cacheStartLine + checks].lastReferencedLine = i;
                        printf("Mark write hit i = %d\n", i);
                        break;
                    }
					printf("setSize = %d and checks = %d\n", setSize, checks);
                    if (setSize == checks+1) {
	                    printf("Mark Write not valid; i = %d\n", i);
	                    for (unsigned int j = 0; j < checks+1; j++) {
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
	                            printf("Mark Write replace; i = %d\n", i);
	                            if (replacementPolicy == 'L') {
                                    int LRU = 0;
                                    for (unsigned int k = 0; k < checks; k++) {
                                        if (cacheLine[cacheStartLine + k].lastReferencedLine > LRU) LRU = k;
                                    }
                                    cacheLine[cacheStartLine + LRU].tag = mmFile[i].tag;
                                    cacheLine[cacheStartLine + LRU].dirty = 1;
                                    cacheLine[cacheStartLine + LRU].firstReferencedLine = i;
                                    cacheLine[cacheStartLine + LRU].lastReferencedLine = i;
		                            sprintf(cacheLine[cacheStartLine + j].data, "mm blk # %d", mmFile[i].mmblk);
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
		                            sprintf(cacheLine[cacheStartLine + j].data, "mm blk # %d", mmFile[i].mmblk);
		                            mmFile[i].hitmiss = 0;
	                            }
                            }
                        }
                    }
                }
            }
        }

        //printf("Mark End\n");
        output(mmFile, cacheLine, addressLines, offset, index, tagSize, cacheSections);
        printf("\nContinue? (y = yes, n = no); ");
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
    int hitsPoss=0, hits=0, hitArray[mainMemorySize/blockSize];
	for(int i=0;i < mainMemorySize/blockSize;i++) hitArray[i]=0;
	for(int i=0;i < references;i++) {
		hitArray[mmFile[i].mmblk]++;
		if (mmFile[i].hitmiss == 1) hits++;
	}
	for(int i=0;i < mainMemorySize/blockSize;i++)	if (hitArray[i] > 1) hitsPoss = hitsPoss + (hitArray[i]-1);
    float hitsF = hits, hitsPossF = hitsPoss, referencesF = references;
	printf("\n\nHighest possible hit rate = %d/%d = %2.0f%%\n", hitsPoss, references, hitsPossF/referencesF*100.0);
	printf("Actual hit rate = %d/%d = %2.0f%%\n\n", hits, references, hitsF/referencesF*100.0);
	printf("Final \"status\" of the cache:\nCache blk #\t\tdirty bit\t\tvalid bit\ttag\t\t\tData\n");
	printf("--------------------------------------------------------------\n");
	for(int i=0;i < cacheSize/blockSize;i++){
		printf("\t%d\t\t\t\t%d\t\t\t\t%d\t\t", i, cacheLine[i].dirty, cacheLine[i].valid);
		if (cacheLine[i].valid == 0){
			for(int k=0;k<tagSize;k++) printf("x");
			printf("\t\t\t");
			for(int k=0;k<tagSize;k++) printf("x");
			printf("\n");
		}
		else{
			printf("%d\t\t%s\n", cacheLine[i].tag, cacheLine[i].data);
		}
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