#ifndef FS_INDEXED_H
#define FS_INDEXED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 1024
#define TOTAL_BLOCKS 64
#define MAX_FILES 10
#define MAX_FILENAME_LEN 32

// Max number of int block pointers that can fit in one index block
#define MAX_INDEX_ENTRIES (BLOCK_SIZE / (int)sizeof(int))

/*
block structure - each block has:
1. raw storage bytes
2. its block number
block[TOTAL_BLOCKS] is the simulated disk in this lab
*/
typedef struct Block {
    unsigned char data[BLOCK_SIZE];
    int blockNumber;
} Block;

// Free block linked-list node
typedef struct FreeBlockNode {
    int blockNumber;
    struct FreeBlockNode *next;
} FreeBlockNode;

// File Information Block (FIB) - holds the information needed to find the file data
typedef struct FIB {
    int fibID;
    char fileName[MAX_FILENAME_LEN];
    int fileSize;       // bytes
    int blockCount;     // number of data blocks
    int indexBlock;     // block number of index block
    int inUse;          // helper flag to determine which entries are active and unused
} FIB;

// File system structure
typedef struct FileSystem {
    /* 1.Linked list of free blocks 
    (1) allocating blocks removes them from the head
    (2) returning a free block adds it to the tail
    */
    FreeBlockNode *freeHead;
    FreeBlockNode *freeTail;

    // 2.logical disk blocks available in the volume simulated as array of Blocks
    Block blocks[TOTAL_BLOCKS];

    // 3.file control blocks ID and their status
    int fibStatus[MAX_FILES];

    // 4.list of files (represented as FIB) created
    FIB files[MAX_FILES];

    // 5.number of files created
    int fileCount;

    //quick find amount of free blocks
    int freeBlockCount;
} FileSystem;

// Global file system instance
static FileSystem fs;

// function declarations
void initFS(void);
int createFile(const char *filename, int size);
int deleteFile(const char *filename);
void listFiles(void);
int allocateFreeBlock(void);
void returnFreeBlock(int blockNumber);
void printFreeBlocks(void);
void destroyFS(void);

#endif // FS_INDEXED_H