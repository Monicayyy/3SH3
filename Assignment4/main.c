#include "fs_indexed.h"

int main(void)
{
    initFS();

    createFile("alpha.txt", 3072);
    createFile("beta.txt", 5120);
    printf("\n");

    listFiles();
    printf("\n");
    printFreeBlocks();

    deleteFile("alpha.txt");
    printf("\n");

    listFiles();
    printf("\n");
    printFreeBlocks();

    createFile("gamma.txt", 4096);
    createFile("delta.txt", 8192);
    printf("\n");

    listFiles();
    printf("\n");
    printFreeBlocks();

    deleteFile("beta.txt");
    deleteFile("gamma.txt");
    deleteFile("delta.txt");
    printf("\n");

    listFiles();
    printf("\n");
    printFreeBlocks();

    destroyFS();
    return 0;
}