#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define BUFFER_SIZE 10
#define OFFSET_MASK 0xFFF // 4095 = 2^12 - 1
#define PAGES 8
#define OFFSET_BITS 12
#define PAGE_SIZE 4096 // 4kb

int main(void)
{
    FILE *fptr;
    char buff[BUFFER_SIZE];

    // page table given from page 0-7 --> frame numb
    int page_table[PAGES] = {6, 4, 3, 7, 0, 1, 2, 5};

    // open input file
    fptr = fopen("labaddr.txt", "r");

    // read all logical address in the file
    while (fgets(buff, BUFFER_SIZE, fptr) != NULL)
    {
        int logical_address;
        int page_number;
        int offset;
        int frame_number;
        int physical_address;

        // Convert string to integer
        logical_address = (int)atoi(buff);

        // 12 least significant bits are offset, shift them and get the page number
        page_number = logical_address >> OFFSET_BITS;

        // extract 12 least significant bits by using 0xFFF
        offset = logical_address & OFFSET_MASK;

        // get frame number from page table
        frame_number = (int)page_table[page_number];

        // compute physical address
        physical_address = (frame_number << OFFSET_BITS) | offset;

        printf("Virtual addr is %u: Page# = %u & Offset = %u. Physical addr = %u.\n", logical_address, page_number, offset, physical_address);
    }

    // close the file
    fclose(fptr);
    return 0;
}