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
int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        printf("Usage: %s <reference-sequence-file.txt> [frame_count] [PRA]\n", argv[0]);
        return 1;
    }

    int frame_count = 256;
    const char *pra = "fifo";

    if (argc == 3) {
        if (strcmp(argv[2], "fifo") == 0 || strcmp(argv[2], "lru") == 0 ||
            strcmp(argv[2], "opt") == 0) {
            pra = argv[2];
        } else {
            frame_count = atoi(argv[2]);
        }
    } else if (argc == 4) {
        frame_count = atoi(argv[2]);
        pra = argv[3];
    }

    (void)frame_count;
    (void)pra;

    return 0;
}

/* 
 * Parse the file provided in command line argv[1]
 * 
 */
int parse_file(const char *filename, int **addresses, size_t *address_count) {
    FILE *fp = fopen(filename, "r");
    size_t capacity = 128;
    size_t count = 0;
    int *list = malloc(capacity * sizeof(*list));
    int logical_address;

    if (fp == NULL) {
        perror("Could not open reference sequence file");
        return 1;
    }

    if (list == NULL) {
        perror("Could not allocate address list");
        fclose(fp);
        return 1;
    }

    while (fscanf(fp, "%d", &logical_address) == 1) {
        if (count == capacity) {
            capacity *= 2;
            int *resized_list = realloc(list, capacity * sizeof(*list));

            if (resized_list == NULL) {
                perror("Could not expand address list");
                free(list);
                fclose(fp);
                return 1;
            }

            list = resized_list;
        }

        list[count] = logical_address;
        count++;
    }

    if (!feof(fp)) {
        fprintf(stderr, "Invalid address in reference sequence file: %s\n", filename);
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
 *
 */
// Source - https://stackoverflow.com/a/2566664
// Posted by quinmars
// Retrieved 2026-10-29, License - CC BY-SA 2.5


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

