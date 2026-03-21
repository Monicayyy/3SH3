#ifndef FS_INDEXED_H
#define FS_INDEXED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Lab specifications */
#define BLOCK_SIZE 1024
#define TOTAL_BLOCKS 64
#define MAX_FILES 10
#define MAX_FILENAME_LEN 32

/* Max number of int block pointers that can fit in one index block */
#define MAX_INDEX_ENTRIES (BLOCK_SIZE / (int)sizeof(int))

/* A block may be simulated as such (per lab handout) */
typedef struct Block {
    unsigned char data[BLOCK_SIZE];
    int blockNumber;
} Block;

/* Free block linked-list node */
typedef struct FreeBlockNode {
    int blockNumber;
    struct FreeBlockNode *next;
} FreeBlockNode;

/* File Information Block (FIB) */
typedef struct FIB {
    int fibID;
    char fileName[MAX_FILENAME_LEN];
    int fileSize;       /* bytes */
    int blockCount;     /* number of data blocks */
    int indexBlock;     /* block number of index block */
    int inUse;
} FIB;

/* Volume / file system structure */
typedef struct FileSystem {
    /* 1. free block list simulated as linked list of free blocks */
    FreeBlockNode *freeHead;
    FreeBlockNode *freeTail;

    /* 2. logical disk blocks available in the volume simulated as array of Blocks */
    Block blocks[TOTAL_BLOCKS];

    /* 3. file control blocks ID and their status */
    int fibStatus[MAX_FILES];

    /* 4. list of files (represented as FIB) created */
    FIB files[MAX_FILES];

    /* 5. number of files created */
    int fileCount;

    /* extra convenience metadata */
    int freeBlockCount;
} FileSystem;

/* Global file system instance */
static FileSystem fs;

/* Internal helper: make a new free-list node */
static FreeBlockNode *createFreeBlockNode(int blockNumber)
{
    FreeBlockNode *node = (FreeBlockNode *)malloc(sizeof(FreeBlockNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed for free block node.\n");
        exit(EXIT_FAILURE);
    }

    node->blockNumber = blockNumber;
    node->next = NULL;
    return node;
}

/* Internal helper: find a file by name, return index in fs.files or -1 */
static int findFileIndexByName(const char *filename)
{
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (fs.files[i].inUse && strcmp(fs.files[i].fileName, filename) == 0) {
            return i;
        }
    }
    return -1;
}

/* Internal helper: get first free FIB ID, or -1 if none */
static int getFreeFIBID(void)
{
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (fs.fibStatus[i] == 0) {
            return i;
        }
    }
    return -1;
}

/* Internal helper: clear a block's contents */
static void clearBlock(int blockNumber)
{
    memset(fs.blocks[blockNumber].data, 0, BLOCK_SIZE);
}

/* Utility function:
   removes a free block from the head of the free block list and returns it */
static int allocateFreeBlock(void)
{
    FreeBlockNode *temp;
    int blockNumber;

    if (fs.freeHead == NULL) {
        return -1;
    }

    temp = fs.freeHead;
    blockNumber = temp->blockNumber;
    fs.freeHead = fs.freeHead->next;

    if (fs.freeHead == NULL) {
        fs.freeTail = NULL;
    }

    free(temp);
    fs.freeBlockCount--;

    return blockNumber;
}

/* Utility function:
   adds a free block back to the tail of the free block list */
static void returnFreeBlock(int blockNumber)
{
    FreeBlockNode *node = createFreeBlockNode(blockNumber);

    clearBlock(blockNumber);

    if (fs.freeTail == NULL) {
        fs.freeHead = node;
        fs.freeTail = node;
    } else {
        fs.freeTail->next = node;
        fs.freeTail = node;
    }

    fs.freeBlockCount++;
}

/* Initializes the file system */
static void initFS(void)
{
    int i;
    FreeBlockNode *node;

    fs.freeHead = NULL;
    fs.freeTail = NULL;
    fs.fileCount = 0;
    fs.freeBlockCount = 0;

    for (i = 0; i < TOTAL_BLOCKS; i++) {
        fs.blocks[i].blockNumber = i;
        memset(fs.blocks[i].data, 0, BLOCK_SIZE);
    }

    for (i = 0; i < MAX_FILES; i++) {
        fs.fibStatus[i] = 0;
        fs.files[i].fibID = i;
        fs.files[i].fileName[0] = '\0';
        fs.files[i].fileSize = 0;
        fs.files[i].blockCount = 0;
        fs.files[i].indexBlock = -1;
        fs.files[i].inUse = 0;
    }

    for (i = 0; i < TOTAL_BLOCKS; i++) {
        node = createFreeBlockNode(i);

        if (fs.freeTail == NULL) {
            fs.freeHead = node;
            fs.freeTail = node;
        } else {
            fs.freeTail->next = node;
            fs.freeTail = node;
        }

        fs.freeBlockCount++;
    }

    printf("Filesystem initialized with %d blocks of %d bytes each.\n",
           TOTAL_BLOCKS, BLOCK_SIZE);
}

/* Creates a file with indexed allocation */
static int createFile(const char *filename, int size)
{
    int fibID;
    int dataBlocksNeeded;
    int totalBlocksNeeded;
    int indexBlockNum;
    int *indexEntries;
    int i;

    if (filename == NULL || size < 0) {
        printf("Error: Invalid file name or size.\n");
        return 0;
    }

    if ((int)strlen(filename) >= MAX_FILENAME_LEN) {
        printf("Error: File name '%s' is too long.\n", filename);
        return 0;
    }

    if (findFileIndexByName(filename) != -1) {
        printf("Error: File '%s' already exists.\n", filename);
        return 0;
    }

    if (fs.fileCount >= MAX_FILES) {
        printf("Error: Maximum file limit reached.\n");
        return 0;
    }

    fibID = getFreeFIBID();
    if (fibID == -1) {
        printf("Error: No free FIB available.\n");
        return 0;
    }

    dataBlocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    totalBlocksNeeded = dataBlocksNeeded + 1; /* +1 for index block */

    if (dataBlocksNeeded > MAX_INDEX_ENTRIES) {
        printf("Error: File '%s' is too large for a single index block.\n", filename);
        return 0;
    }

    if (fs.freeBlockCount < totalBlocksNeeded) {
        printf("Error: Not enough free blocks to create '%s'.\n", filename);
        return 0;
    }

    indexBlockNum = allocateFreeBlock();
    if (indexBlockNum == -1) {
        printf("Error: Could not allocate index block for '%s'.\n", filename);
        return 0;
    }

    clearBlock(indexBlockNum);
    indexEntries = (int *)fs.blocks[indexBlockNum].data;

    for (i = 0; i < dataBlocksNeeded; i++) {
        int dataBlockNum = allocateFreeBlock();
        if (dataBlockNum == -1) {
            int j;
            for (j = 0; j < i; j++) {
                returnFreeBlock(indexEntries[j]);
            }
            returnFreeBlock(indexBlockNum);
            printf("Error: Failed during block allocation for '%s'.\n", filename);
            return 0;
        }

        indexEntries[i] = dataBlockNum;
        clearBlock(dataBlockNum);
    }

    fs.files[fibID].fibID = fibID;
    strcpy(fs.files[fibID].fileName, filename);
    fs.files[fibID].fileSize = size;
    fs.files[fibID].blockCount = dataBlocksNeeded;
    fs.files[fibID].indexBlock = indexBlockNum;
    fs.files[fibID].inUse = 1;

    fs.fibStatus[fibID] = 1;
    fs.fileCount++;

    printf("File '%s' created with %d data blocks + 1 index block.\n",
           filename, dataBlocksNeeded);

    return 1;
}

/* Deletes a file by name
   Important: return data blocks first, then index block last,
   so the free-list order matches your expected output. */
static int deleteFile(const char *filename)
{
    int fileIndex;
    int indexBlockNum;
    int *indexEntries;
    int i;

    fileIndex = findFileIndexByName(filename);
    if (fileIndex == -1) {
        printf("Error: File '%s' not found.\n", filename);
        return 0;
    }

    indexBlockNum = fs.files[fileIndex].indexBlock;
    indexEntries = (int *)fs.blocks[indexBlockNum].data;

    /* Return data blocks first */
    for (i = 0; i < fs.files[fileIndex].blockCount; i++) {
        returnFreeBlock(indexEntries[i]);
    }

    /* Return index block last */
    returnFreeBlock(indexBlockNum);

    fs.fibStatus[fs.files[fileIndex].fibID] = 0;
    fs.files[fileIndex].fileName[0] = '\0';
    fs.files[fileIndex].fileSize = 0;
    fs.files[fileIndex].blockCount = 0;
    fs.files[fileIndex].indexBlock = -1;
    fs.files[fileIndex].inUse = 0;

    fs.fileCount--;

    printf("File '%s' deleted.\n", filename);
    return 1;
}

/* Lists all files in the flat file system */
static void listFiles(void)
{
    int i;

    printf("\nRoot Directory Listing (%d files):\n", fs.fileCount);

    for (i = 0; i < MAX_FILES; i++) {
        if (fs.files[i].inUse) {
            printf("  %-10s | %6d bytes | %2d data blocks | FIBID=%d\n",
                   fs.files[i].fileName,
                   fs.files[i].fileSize,
                   fs.files[i].blockCount,
                   fs.files[i].fibID);
        }
    }
}

/* Displays all free block numbers and total count */
static void printFreeBlocks(void)
{
    FreeBlockNode *current = fs.freeHead;

    printf("\nFree Blocks (%d): ", fs.freeBlockCount);

    while (current != NULL) {
        printf("[%d] -> ", current->blockNumber);
        current = current->next;
    }

    printf("NULL\n");
}

/* Optional cleanup helper for program exit */
static void destroyFS(void)
{
    FreeBlockNode *current = fs.freeHead;
    FreeBlockNode *nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }

    fs.freeHead = NULL;
    fs.freeTail = NULL;
    fs.freeBlockCount = 0;
}

#endif /* FS_INDEXED_H */