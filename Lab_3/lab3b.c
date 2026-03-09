#include <stdio.h>
#include <sys/mman.h>   /* mmap(), munmap() */
#include <string.h>     /* memcpy() */
#include <fcntl.h>      /* open() */
#include <stdlib.h>
#include <unistd.h>     /* close() */

#define INT_SIZE 4
#define INT_COUNT 10
#define MEMORY_SIZE (INT_COUNT * INT_SIZE)

int intArray[INT_COUNT];
signed char *mmapfptr;

int main(void)
{
    int mmapfile_fd;
    int i;
    int sum = 0;

    /* Open the binary file for reading */
    mmapfile_fd = open("numbers.bin", O_RDONLY);
    if (mmapfile_fd < 0) {
        perror("open");
        return 1;
    }

    /* Memory-map the file */
    mmapfptr = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, mmapfile_fd, 0);
    if (mmapfptr == MAP_FAILED) {
        perror("mmap");
        close(mmapfile_fd);
        return 1;
    }

    /* Copy one integer (4 bytes) at a time into intArray */
    for (i = 0; i < INT_COUNT; i++) {
        memcpy(intArray + i, mmapfptr + (INT_SIZE * i), INT_SIZE);
    }

    /* Unmap the file */
    if (munmap(mmapfptr, MEMORY_SIZE) == -1) {
        perror("munmap");
        close(mmapfile_fd);
        return 1;
    }

    /* Close the file descriptor */
    close(mmapfile_fd);

    /* Add all integers in the array */
    for (i = 0; i < INT_COUNT; i++) {
        sum += intArray[i];
    }

    printf("Sum of numbers = %d\n", sum);

    return 0;
}