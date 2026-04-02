#include "fs_indexed.h"

int main(void)
{
    initFS();
    printf("\n");

    printFreeBlocks();
    printf("\n");

    createFile("alpha.txt", 3072);
    listFiles();
    printFreeBlocks();
    printf("\n");

    createFile("beta.txt", 5120);
    listFiles();
    printFreeBlocks();
    printf("\n");

    createFile("gamma.txt", 1500);
    listFiles();
    printFreeBlocks();
    printf("\n");

    deleteFile("beta.txt");
    listFiles();
    printFreeBlocks();
    printf("\n");

    deleteFile("alpha.txt");
    listFiles();
    printFreeBlocks();
    printf("\n");

    destroyFS();
    return 0;
}
