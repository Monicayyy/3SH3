#include <stdio.h>
#include <sys/mman.h>   /*For mmap() function*/
#include <string.h>     /*For memcpy() function*/
#include <fcntl.h>      /*For file descriptors: open()*/
#include <stdlib.h>      /*For file descriptors*/

#define INT_SIZE 4 // Size of integer in bytes
#define INT_COUNT 10 
#define MEMORY_SIZE (INT_COUNT * INT_SIZE)

int intArray[INT_COUNT];
signed char *mmapfptr;

int main(void)
{
    int mmapfile_fd;
    int i;
    int sum = 0;

    /* Open the binary file for reading*/
    mmapfile_fd = open("numbers.bin", O_RDONLY);
 
    /* Memory-map the file*/
    mmapfptr = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, mmapfile_fd, 0);

    /* Copy one integer (4 bytes) at a time into intArray */
    for (i = 0; i < INT_COUNT; i++) {
        memcpy(intArray + i, mmapfptr + (INT_SIZE * i), INT_SIZE);
    }

    /* Unmap the file */
    munmap(mmapfptr, MEMORY_SIZE);

    /* Add all integers in the array */
    for (i = 0; i < INT_COUNT; i++) {
        sum += intArray[i];
    }

    printf("Sum of numbers = %d\n", sum);

    return 0;
}
