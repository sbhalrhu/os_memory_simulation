#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PAGE_TABLE_ENTRIES 256
#define TLB_ENTRIES 16
#define PAGE_SIZE_BYTES 256
#define FRAME_SIZE_BYTES PAGE_SIZE_BYTES


/*-------------------------------------------STRUCTURE DEFINITIONS-------------------------------------------*/

/* TLB: Tracks a few of the most recently accessed addresses.
 * One entry contains LOGICAL page # and corresponding PHYSICAL frame #.
 * Entries are replaced on a FIFO basis. 
 * Do NOT give priority to currently empty entries.
 * TLB size is fixed (macro: TLB_ENTRIES).
 *
 *
 * Page Table: Tracks EVERY page in memory. 
 * One entry contains LOGICAL page #, PHYSICAL frame #.
 * After checking the TLB, the page table is checked.
 * If the page is found, access it in the corresponding physical frame.
 * If the page is not found, PAGE FAULT:
 * PAGE FAULT: Fetch a page from the backing store and load it into a physical frame.
 * Use PRA to update page table and TLB accordingly.
 * 
 * 
 * 
 * Backing store / disk: Stores content of all pages.
 * This is PROVIDED and is in a file called BACKING_STORE.bin.
 * Size of provided file is fixed (macro: PAGE_SIZE_BYTES * macro: PAGE_TABLE_ENTRIES).
 * For us it is 65,536 bytes.
 * Open the file using system calls and read from it as binary (fopen, fread).
 * 
 * 
 * Physical Memory / RAM: Contains the content of memory.
 * Split into frames with fixed size.
 * Frame count is variable depending on command line argument.
 * Frame SIZE is fixed to be same as page size (macro: FRAME_SIZE_BYTES).
 * Since page count > frame count, ...
 * ...use PRA to determine what pages are loaded into physical frames.
 * 
 * 
 * 
 * Reference sequence: A list of logical addresses to be translated to physical addresses.
 * This is provided in the file in command line argument argv[1].
 * 
 * 
 * 
 */







/* Command line usage:      ./memSim <reference-sequence-file.txt> <frame_count> <PRA> 
 * Command line arguments:  
 * argv[0] ./Executable-name
 * argv[1] Reference sequence file 
 * argv[2] Frame count 
 * argv[3] Page replacement algorithm/PRA -- FIFO, LRU, OPT  
 *
 *
 * Output: Print lines to stdout.
 * For each address in the reference sequence, print out one line in stdout with the format:
 * <logical_address>, <value_at_that_address>, <physical_frame_number>, <content_of_entire_frame><new_line_character> 
 * value_at_that_address is ONE SIGNED integer
 * physical_frame_number is ONE POSITIVE integer
 * content_of_entire_frame is 256 BYTES OF ASCII. 
 * Print out bytes as chars (%c).
 *
 */

struct page_entry{
    int load_bit; //1 = loaded
    int frame_num;
};

struct TLB_entry{
    int valid; //1 = valid
    int page_num; 
    int frame_num;
};

int parse_file(const char *filename, int **addresses, int *address_count);


int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        printf("Usage: ./memSim <reference-sequence-file.txt> [frame_count] [PRA]\n");
        return 1;
    }

    /*default values:
    256 frames (also the max value) 
    FIFO replacement (must be fifo, lru, or opt)
    */
   
    int frame_count = 256;
    const char *pra = "fifo";

    /*Frame count and PRA are optional arguments. 
      Defaults are already set.
      Account for three cases: 
      1. no frame count and no PRA [argc = 2]
      2. no PRA [argc = 3]
      3. both frame count and PRA [argc = 4] 
      Case 1 already accounted for by setting defaults.
    */
    if (argc > 2) {
        if (atoi(argv[2]) > 0 && atoi(argv[2]) <= 256) {
            frame_count = atoi(argv[2]);
        }
    }
    if (argc == 4) {
        if (strcmp(argv[3], "fifo") == 0 || strcmp(argv[3], "lru") == 0 ||
            strcmp(argv[3], "opt") == 0) {
            pra = argv[3];
        } 
    }


    
    /*collect address list from txt file*/
    int * addresses;
    int num_addresses = 0;
    int parsing_status = parse_file(argv[1], &addresses, &num_addresses);
    if (parsing_status != 0){
        printf("couldn't parse file\n");
        return 1;
    }

    /*initialize page table, TLB, and physical memory*/
    struct page_entry page_table[PAGE_TABLE_ENTRIES];
    struct TLB_entry TLB[TLB_ENTRIES];
    unsigned char *physical_memory = malloc(FRAME_SIZE_BYTES*frame_count*sizeof(unsigned char));
    if (physical_memory == NULL){
        free(physical_memory);
        free(addresses);
        return 1;
    }

    /*frame to page list, will help with updating page table when a frame gets evicted from memory*/
    int *frame_to_page = malloc(frame_count*sizeof(int)); //when we evict a frame, figure out the page
    if (frame_to_page == NULL){
        free(frame_to_page);
        free(addresses);
        free(physical_memory);
        return 1;
    }

    /*more initialization stuff*/
    for (int i = 0; i < PAGE_TABLE_ENTRIES; i++){
        page_table[i].frame_num = -1;
        page_table[i].load_bit = 0;
    }
    for (int i = 0; i < TLB_ENTRIES; i++){
        TLB[i].frame_num = -1;
        TLB[i].page_num = -1;
        TLB[i].valid = 0;
    }
    for (int i = 0; i < frame_count; i++){
        frame_to_page[i] = -1;
    }

    /*virtual memory startup*/
    FILE *fp = fopen("BACKING_STORE-1.bin", "rb");
    if (fp == NULL) {
        printf("Could not open backing store");
        free(addresses);
        free(physical_memory);
        free(frame_to_page);
        return 1;
    }

    int tlb_next_slot = 0;
    int tlb_hits = 0;
    int tlb_misses = 0;



    int next_free_frame = 0;
    int page_faults = 0;


    /*actual loop to find memory from addresses*/
    for (int i = 0; i < num_addresses; i++){
        int page_num = addresses[i]/256;
        int offset = addresses[i]%256;
        int frame_num;

        int tlb_hit = 0; 
        //tlb check first
        for (int b = 0; b < 16; b++){
            if (TLB[b].valid && TLB[b].page_num == page_num){
                tlb_hits++;
                tlb_hit = 1;
                frame_num = TLB[b].frame_num;
                break;
            }
        }
        /*if the TLB does not hit, look at the page table*/
        if (tlb_hit != 1){
            tlb_misses++;
            //page fault
            if (page_table[page_num].load_bit == 0){
                /*physical memory is full, time for memory replacement algos*/
                if (next_free_frame >= frame_count) {
                    printf("num frames exceeded\n");
                    fclose(fp);
                    free(addresses);
                    free(physical_memory);
                    free(frame_to_page);
                    return 1;
                }
                frame_num = next_free_frame;

                /*look for data in backing store*/
                if (fseek(fp, (long)page_num * PAGE_SIZE_BYTES, SEEK_SET) != 0) {
                    printf("Could not seek in backing store");
                    fclose(fp);
                    free(addresses);
                    free(physical_memory);
                    free(frame_to_page);
                    return 1;
                }

                /*reads backing store data into physical memory*/
                size_t bytes_read = fread(physical_memory + (frame_num * FRAME_SIZE_BYTES), 1, PAGE_SIZE_BYTES, fp);

                if (bytes_read != PAGE_SIZE_BYTES) {
                    printf("Could not read full page from backing store\n");
                    fclose(fp);
                    free(addresses);
                    free(physical_memory);
                    free(frame_to_page);
                    return 1;
                }
                
                /*page table is updated*/
                page_table[page_num].load_bit = 1;
                page_table[page_num].frame_num = frame_num;
                frame_to_page[frame_num] = page_num;

                next_free_frame++;
                page_faults++; 
            }
            else {
                frame_num = page_table[page_num].frame_num;
            }
            /*after page table was checked/modified, modify next available tlb slot via fifo*/
            TLB[tlb_next_slot].valid = 1;
            TLB[tlb_next_slot].page_num = page_num;
            TLB[tlb_next_slot].frame_num = frame_num;
            tlb_next_slot = (tlb_next_slot + 1) % TLB_ENTRIES;
        }
        int physical_index = (frame_num * FRAME_SIZE_BYTES) + offset;
        signed char value = (signed char)physical_memory[physical_index];

        printf("logical=%d page=%d offset=%d frame=%d value=%d\n", addresses[i], page_num, offset, frame_num, value);

    }

    fclose(fp);
    free(addresses);
    free(physical_memory);
    free(frame_to_page);
    return 0;
}

/* 
 * Parse the file provided in command line argv[1]
 * 
 */
int parse_file(const char *filename, int **addresses, int *address_count) {
    FILE *fp = fopen(filename, "r");
    int capacity = 128;
    int count = 0;
    int *list = malloc(capacity * sizeof(*list));
    int logical_address;

    if (fp == NULL) {
        printf("Could not open reference sequence file");
        free(list);
        return 1;
    }

    if (list == NULL) {
        printf("Could not allocate address list");
        free(list);
        fclose(fp);
        return 1;
    }

    while (fscanf(fp, "%d", &logical_address) == 1) {
        if (count == capacity) {
            capacity *= 2;
            int *resized_list = realloc(list, capacity * sizeof(*list));

            if (resized_list == NULL) {
                printf("Could not expand address list");
                free(list);
                fclose(fp);
                return 1;
            }

            list = resized_list;
        }

        list[count] = logical_address;
        count++;
    }

    /*If the while loop terminates before the end of the file is reached, 
      report an error and free list's memory.
    */
    if (!feof(fp)) {
        printf("Invalid address in reference sequence file: %s\n", filename);
        free(list);
        fclose(fp);
        return 1;
    }

    fclose(fp);
    *addresses = list;
    *address_count = count;
    return 0;
}



/* Function found online to 'dump a whole memory region on the screen'.
 
Source - https://stackoverflow.com/a/2566664
Posted by quinmars
Retrieved 2026-10-29, License - CC BY-SA 2.5


void
hexdump(const void *data, int size)
{
    const unsigned char *byte = data;

    while (size > 0)
    {
        size--;
        printf("%.2x ", *byte);
        byte++;
    }
}

*/

