# os_memory_simulation
Working repository for OS project 3: memory simulation

Documentation:
    % operations on logical addresses to find their page/frame number
    To read from a file as binary, fopen and fread()
    
    FILE *fp = fopen("file.txt", "r");
    size_t fread(void *ptr, size_t size, size_t count, FILE *stream);


Add to main() comments:
    Additional stats printed at end. 

    At the end, print: (no indentation)
        Number of Translated Addresses = [count]
        Page Faults = [count]
        Page Fault Rate = [probability (max 1.000, should never reach 0)]
        TLB Hits = [count]
        TLB Misses = [count]
        TLB Hit Rate = [probability (min 0.000, should never reach 1)]

Struct drafts:
struct tlb{ 
    int page_number,
    int frame_number,
    

} tlb;



