// please type: make run
#include "fs_indexed.h"

// global file system instance
static FileSystem fs;

// helper function
// calculate how many block is needed for the file
static int blocksNeeded(int size)
{
    // at least  block
    if (size <= 0) {
        return 0;
    }
    // Round up
    return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

// helper function
// clear the contents of a block
static void clearBlock(int blockNumber)
{
    for (int i = 0; i < BLOCK_SIZE; i++) {
        fs.blocks[blockNumber].data[i] = 0;
    }
}

// helper function
// search for a file by name in the file table
static int findFileIndex(const char *filename)
{
    int i;

    for (i = 0; i < MAX_FILES; i++) {
        // file must be in use and names must match
        if (fs.files[i].inUse && strcmp(fs.files[i].fileName, filename) == 0) {
            return i;
        }
    }

    return -1;
}

// helper function
// get the index of FIB 
static int getFileInformationBlockID(void)
{
    if (fs.nextFibID >= MAX_FILES) {
        return -1;
    }

    fs.fibStatus[fs.nextFibID] = 0;
    return fs.nextFibID++;
}

// helper function
// return a FIB ID back to the pool of available FIB IDs
static void releaseFileInformationBlockID(int fibID)
{
    if (fibID >= 0 && fibID < MAX_FILES) {
        fs.fibStatus[fibID] = 1;
    }
}

// utility function
// allocate one free block from the head of the free block list
int allocateFreeBlock(void)
{
    FreeBlockNode *temp;
    int blockNumber;

    // no free blocks left
    if (fs.freeHead == NULL) {
        return -1;
    }

    // remove node from head of free list
    temp = fs.freeHead;
    blockNumber = temp->blockNumber;
    fs.freeHead = fs.freeHead->next;

    // if list becomes empty, tail must also become NULL
    if (fs.freeHead == NULL) {
        fs.freeTail = NULL;
    }

    // free the linked list node structure
    free(temp);

    // update num of free blocks
    fs.freeBlockCount--;

    // clear block contents before giving it out
    clearBlock(blockNumber);

    return blockNumber;
}


// utility function
// return a block back to the tail of the free block list.
void returnFreeBlock(int blockNumber)
{
    FreeBlockNode *node;

    // create a new free-list node
    node = (FreeBlockNode *)malloc(sizeof(FreeBlockNode));
    if (node == NULL) {
        printf("Error: could not return block %d to free list.\n", blockNumber);
        return;
    }

    // clear old data inside the block
    clearBlock(blockNumber);

    node->blockNumber = blockNumber;
    node->next = NULL;

    // if free list is empty, new node becomes both head and tail
    if (fs.freeTail == NULL) {
        fs.freeHead = node;
        fs.freeTail = node;
    } 
    // if not, just add it to the tail
    else {
        fs.freeTail->next = node;
        fs.freeTail = node;
    }

    fs.freeBlockCount++;
}


void initFS(void)
{
    int i;
    FreeBlockNode *temp;

    // free all free list nodes
    while (fs.freeHead != NULL) {
        temp = fs.freeHead;
        fs.freeHead = fs.freeHead->next;
        free(temp);
    }

    // reset linked list pointers and counters
    fs.freeHead = NULL;
    fs.freeTail = NULL;
    fs.fileCount = 0;
    fs.freeBlockCount = 0;
    fs.nextFibID = 0;

    // initialize all disk blocks
    for (i = 0; i < TOTAL_BLOCKS; i++) {
        fs.blocks[i].blockNumber = i;
        for (int j = 0; j < BLOCK_SIZE; j++) {
            fs.blocks[i].data[j] = 0;
        }
    }

    // initialize all FIB entries and mark them as free
    for (i = 0; i < MAX_FILES; i++) {
        fs.fibStatus[i] = 1;              // 1 means FIB is available
        fs.files[i].fibID = i;
        fs.files[i].fileName[0] = '\0';
        fs.files[i].fileSize = 0;
        fs.files[i].blockCount = 0;
        fs.files[i].indexBlock = -1;
        fs.files[i].inUse = 0;
    }

    // put every block into the free block list
    for (i = 0; i < TOTAL_BLOCKS; i++) {
        FreeBlockNode *newNode = (FreeBlockNode *)malloc(sizeof(FreeBlockNode));
        if (newNode == NULL) {
            printf("Error: file system initialization failed.\n");
            return;
        }

        // initialize
        newNode->blockNumber = i;
        newNode->next = NULL;

        // first new node
        if (fs.freeHead == NULL) {
            fs.freeHead = newNode;
            fs.freeTail = newNode;
        } 
        // the rest
        else {
            fs.freeTail->next = newNode;
            fs.freeTail = newNode;
        }

        fs.freeBlockCount++;
    }

    printf("Filesystem initialized with %d blocks of %d bytes each.\n",
       TOTAL_BLOCKS, BLOCK_SIZE);
}

// create a new file using indexed allocation
int createFile(const char *filename, int size)
{
    int fibID;
    int NumOfDataBlocks;
    int totalBlocksNeeded;
    int indexBlock;
    int *indexEntries;
    int allocated[MAX_INDEX_ENTRIES];
    int i;

    // Validate filename
    if (filename == NULL || strlen(filename) == 0 || strlen(filename) >= MAX_FILENAME_LEN) {
        printf("Error: invalid filename.\n");
        return 0;
    }

    // calculate required number of data blocks
    NumOfDataBlocks = blocksNeeded(size);

    /* One index block can only hold a limited number of block pointers */
    if (NumOfDataBlocks > MAX_INDEX_ENTRIES) {
        printf("Error: file '%s' is too large for one index block.\n", filename);
        return 0;
    }

    // need 1 index block plus the data blocks
    totalBlocksNeeded = NumOfDataBlocks + 1;

    // make sure enough free blocks exist
    if (totalBlocksNeeded > fs.freeBlockCount) {
        printf("Error: not enough free blocks for '%s'.\n", filename);
        return 0;
    }

    // find an empty FIB ID for this file
    fibID = getFileInformationBlockID();
    if (fibID == -1) {
        printf("Error: no free FIB ID available.\n");
        return 0;
    }

    // allocate the index block
    indexBlock = allocateFreeBlock();
    if (indexBlock == -1) {
        releaseFileInformationBlockID(fibID);
        printf("Error: failed to allocate index block.\n");
        return 0;
    }

    // make the block to store index as int
    indexEntries = (int *)fs.blocks[indexBlock].data;

    // initialize all index entries to -1
    for (i = 0; i < MAX_INDEX_ENTRIES; i++) {
        indexEntries[i] = -1;
    }

    // allocate all needed data blocks
    for (i = 0; i < NumOfDataBlocks; i++) {
        allocated[i] = allocateFreeBlock();

        // if allocation fails midway, clean up everything already allocated
        if (allocated[i] == -1) {
            int j;

            for (j = 0; j < i; j++) {
                returnFreeBlock(allocated[j]);
            }

            returnFreeBlock(indexBlock);
            releaseFileInformationBlockID(fibID);

            printf("Error: failed to allocate data blocks.\n");
            return 0;
        }

        // store the allocated data block number inside the index block
        indexEntries[i] = allocated[i];
    }

    // fill the file's information in its FIB
    fs.files[fibID].fibID = fibID;
    strcpy(fs.files[fibID].fileName, filename);
    fs.files[fibID].fileSize = size;
    fs.files[fibID].blockCount = NumOfDataBlocks;
    fs.files[fibID].indexBlock = indexBlock;
    fs.files[fibID].inUse = 1;

    fs.fileCount++;

    printf("File '%s' created with %d data blocks + 1 index block.\n",
       filename, NumOfDataBlocks);
    return 1;
}

// delete a file from the file system
int deleteFile(const char *filename)
{
    int fileIndex;
    int indexBlock;
    int *indexEntries;
    int i;

    // search for the file
    fileIndex = findFileIndex(filename);
    if (fileIndex == -1) {
        printf("Error: file '%s' not found.\n", filename);
        return 0;
    }

    // get the file's index block
    indexBlock = fs.files[fileIndex].indexBlock;

    // read the block numbers stored in the index block
    indexEntries = (int *)fs.blocks[indexBlock].data;

    // return each data block used by the file in index block
    for (i = 0; i < fs.files[fileIndex].blockCount; i++) {
        if (indexEntries[i] != -1) {
            returnFreeBlock(indexEntries[i]);
        }
    }

    // return the index block itself
    returnFreeBlock(indexBlock);

    // release the FIB ID so it can be reused
    releaseFileInformationBlockID(fs.files[fileIndex].fibID);

    // Clear the file's FIB entry
    fs.files[fileIndex].fileName[0] = '\0';
    fs.files[fileIndex].fileSize = 0;
    fs.files[fileIndex].blockCount = 0;
    fs.files[fileIndex].indexBlock = -1;
    fs.files[fileIndex].inUse = 0;

    fs.fileCount--;

    printf("File '%s' deleted.\n", filename);
    return 1;
}


// display all files currently stored in the root directory.
// shows: file name, file size in bytes, number of data blocks, FIB ID
void listFiles(void)
{
    int i;

    printf("Root Directory Listing (%d files):\n", fs.fileCount);

    for (i = 0; i < MAX_FILES; i++) {
        if (fs.files[i].inUse) {
            printf("  %-10s | %5d bytes | %d data blocks | FIBID=%d\n",
                   fs.files[i].fileName,
                   fs.files[i].fileSize,
                   fs.files[i].blockCount,
                   fs.files[i].fibID);
        }
    }
}

// print the current free block list from head to tail
void printFreeBlocks(void)
{
    FreeBlockNode *current = fs.freeHead;

    printf("Free Blocks (%d): ", fs.freeBlockCount);

    while (current != NULL) {
        printf("[%d]", current->blockNumber);
        if (current->next != NULL) {
            printf(" -> ");
        }
        current = current->next;
    }
    printf("\n");
}

void destroyFS(void)
{
    FreeBlockNode *temp;

    // free free list
    while (fs.freeHead != NULL) {
        temp = fs.freeHead;
        fs.freeHead = fs.freeHead->next;
        free(temp);
    }

    fs.freeHead = NULL;
    fs.freeTail = NULL;

    // reset counters
    fs.fileCount = 0;
    fs.freeBlockCount = 0;
}