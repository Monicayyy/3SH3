// terminal command: 
// gcc assignment3.c -o assignment3
// ./assignment3

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 10
#define OFFSET_MASK 0xFF // 255 = 2^8 - 1
#define OFFSET_BITS 8
#define PAGE_SIZE 256 // 256 = 2^8
#define PAGE_TOTAL 256  // 2^16 / 2^8 = 256 pages (total pages)
#define FRAME_TOTAL 128 // 2^15 / 2^8 = 128 frames
#define TLB_SIZE 16

int main(void)
{
    // OPEN address.txt
    FILE *fptr;
    char buff[BUFFER_SIZE];

    fptr = fopen("addresses.txt", "r");
    if (fptr == NULL)
    {
        perror("CANT open addresses.txt");
        return 1;
    }


    // CREATE a page to frame table and a frame to page table
    int page_table[PAGE_TOTAL]; // 256 items
    int frame_to_page[FRAME_TOTAL]; // 128 items

    // Initialize page table with -1
    for (int i = 0; i < PAGE_TOTAL; i++)
    {
        page_table[i] = -1;
    }

    // Initialize frame to page table with -1
    for (int i = 0; i < FRAME_TOTAL; i++)
    {
        frame_to_page[i] = -1;
    }


    // CREATE a pointer for backing store
    signed char physical_memory[FRAME_TOTAL * PAGE_SIZE];
    int backing_bin;
    signed char *backing;

    // open backing store
    backing_bin = open("BACKING_STORE.bin", O_RDONLY);
    if (backing_bin < 0)
    {
        perror("Cannot open BACKING_STORE.bin");
        fclose(fptr);
        return 1;
    }

    // map BACKING_STORE.bin into memory and let backing pointing to it
    backing = mmap(0, PAGE_TOTAL * PAGE_SIZE, PROT_READ, MAP_PRIVATE, backing_bin, 0);
    if (backing == MAP_FAILED)
    {
        perror("mmap failed");
        close(backing_bin);
        fclose(fptr);
        return 1;
    }


    // CREATE the TLB
    int tlb_page[TLB_SIZE];
    int tlb_frame[TLB_SIZE];
    int tlb_index = 0;
    int tlb_hit = 0;

    // initialize the TLB with -1
    for (int i = 0; i < TLB_SIZE; i++)
    {
        tlb_page[i] = -1;
        tlb_frame[i] = -1;
    }

    int next_free_frame = 0;
    int fifo_index = 0;
    int page_fault_count = 0;
    int total_addresses = 0;

    while (fgets(buff, BUFFER_SIZE, fptr) != NULL)
    {
        int logical_address;
        int page_number;
        int offset;
        int frame_number;
        int physical_address;
        signed char data;
        int tlb_found = 0;

        logical_address = atoi(buff);

        // 8 least significant bits are offset, shift them and get the page number
        page_number = logical_address >> OFFSET_BITS;
        // extract 8 least significant bits by using 0xFF
        offset = logical_address & OFFSET_MASK;

        // check whether the page number is in the TLB, yes -> hit
        for (int i = 0; i < TLB_SIZE; i++)
        {
            if (tlb_page[i] == page_number)
            {
                frame_number = tlb_frame[i];
                tlb_hit++;
                tlb_found = 1;
                break;
            }
        }

        // if it is not in TLB
        if (!tlb_found)
        {
            // Look up page table
            // cant find in the table, page fault
            if (page_table[page_number] == -1)
            {
                page_fault_count++;
                
                // there are still empty frames in physical memory
                if (next_free_frame < FRAME_TOTAL)
                {
                    frame_number = next_free_frame;
                    next_free_frame++;
                }
                else
                {   
                    // physical memory is full, need to do FIFO replacement
                    int victim_page;

                    frame_number = fifo_index;
                    victim_page = frame_to_page[frame_number];

                    // kick the first-in page
                    page_table[victim_page] = -1;
                    
                    // find the next first-in page, circular
                    fifo_index = (fifo_index + 1) % FRAME_TOTAL;
                }

                // copy 256 bytes from backing store to physical memory
                memcpy(physical_memory + frame_number * PAGE_SIZE,
                    backing + page_number * PAGE_SIZE,
                    PAGE_SIZE);

                page_table[page_number] = frame_number;
                frame_to_page[frame_number] = page_number;
            }
            frame_number = page_table[page_number];

            // update TLB
            tlb_page[tlb_index] = page_number;
            tlb_frame[tlb_index] = frame_number;
            tlb_index = (tlb_index + 1) % TLB_SIZE;
        }
        // TLB hit or page table hit or or page fault already handled
        physical_address = (frame_number << OFFSET_BITS) | offset;
        data = physical_memory[frame_number * PAGE_SIZE + offset];
        total_addresses++;

        printf("Virtual address: %d Physical address = %d Value=%d\n",
        logical_address, physical_address, data);
    }
    printf("Total addresses = %d\n", total_addresses);
    printf("Page faults = %d\n", page_fault_count);
    printf("TLB Hits = %d\n", tlb_hit);

    munmap(backing, PAGE_TOTAL * PAGE_SIZE);
    close(backing_bin);

    fclose(fptr);
    return 0;
}