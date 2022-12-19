#include "paging.h"

page_directory_t page_directory[ONE_K] __attribute__ ((aligned(FOUR_K)));
page_table_t page_table[ONE_K]  __attribute__ ((aligned(FOUR_K)));
page_table_t page_table_video[ONE_K] __attribute__ ((aligned(FOUR_K)));
/* 
 *paging_int
 * DESCRIPTION: initialize paging
 * INPUTS: None
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: Initialize the Page Directory, Page Table
 */
void paging_init(){
    int i;
    // initialize the page directory entry
    for(i = 0; i < PAGE_SIZE; i++){
        page_directory[i].p = 0;
        page_directory[i].rw = 1;   // set Read/write (R/W) flag to be 1
        page_directory[i].us = 0;
        page_directory[i].pwt = 0;
        page_directory[i].pcd = 0;
        page_directory[i].a = 0;
        page_directory[i].reserved = 0;
        page_directory[i].ps =0;
        page_directory[i].g = 0;
        page_directory[i].avail = 0;
    }
    // initialize the page table entry
    for(i = 0; i < PAGE_SIZE; i++){
        page_table[i].p = 0;
        page_table[i].rw = 1;       // set Read/write (R/W) flag to be 1
        page_table[i].us = 0;
        page_table[i].pwt = 0;
        page_table[i].pcd = 0;
        page_table[i].a = 0;
        page_table[i].d = 0;
        page_table[i].pat =0;
        page_table[i].g = 0;
        page_table[i].avail = 0;
        page_table[i].addr = i;
    }
    // initialize the page table for video memory
    for(i=0;i<PAGE_SIZE;i++){
        page_table_video[i].p = 0;
        page_table_video[i].rw = 1;  // set Read/write (R/W) flag to be 1
        page_table_video[i].us = 1;
        page_table_video[i].pwt = 0;
        page_table_video[i].pcd = 0;
        page_table_video[i].a = 0;
        page_table_video[i].d = 0;
        page_table_video[i].pat =0;
        page_table_video[i].g = 0;
        page_table_video[i].avail = 0;
        page_table_video[i].addr = i;
    }
    page_table[184].p = 1;      // set the video memory virtual address 0XB8000                                  ; B8 is equivalent to 184
    page_table[184].us = 1;
    page_table[185].p = 1;      // set the video memory (for back buffer of terminal 1) virtual address 0XB9000  ; B9 is equivalent to 185
    page_table[185].us = 1;
    page_table[186].p = 1;      // set the video memory (for back buffer of terminal 2) virtual address 0XBA000  ; BA is equivalent to 186
    page_table[186].us = 1;
    page_table[187].p = 1;      // set the video memory (for back buffer of terminal 3) virtual address 0XBB000  ; BB is equivalent to 187
    page_table[187].us = 1;
    // page directory entry is 0(0MB - 4MB) is for video mem
    page_directory[0].p = 1;
    page_directory[0].addr = (unsigned int)page_table >> 12;  // page directory entry 0 point to page table
    // page directory entry is 1(4MB - 8MB) is for Kernel
    page_directory[1].p = 1;
    page_directory[1].ps = 1;   // set the page size to be 4MB
    page_directory[1].g = 1;    // set it to be a global page
    page_directory[1].addr = NUMBER_4MB >> 12;
    // enable paging
    enable_paging((void*)page_directory);
}

/* 
 * map_program
 * DESCRIPTION: This function is used to load in the page and map the program.
 * INPUTS: pid - process number
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: None
 */
void map_program(uint32_t pid){
    // physical memory starts at 8MB + (process number * 4MB)
    uint32_t addr = NUMBER_8MB + pid * NUMBER_4MB; //8MB + pid*4MB
    page_directory[USER_PAGE_NUM].p = 1;  // set Present (P) flag to be 1. Indicates whether the page or page
    page_directory[USER_PAGE_NUM].rw = 1; // set Read/write (R/W) flag to be 1
    page_directory[USER_PAGE_NUM].us = 1; // set User/supervisor (U/S) flag to be 1
    page_directory[USER_PAGE_NUM].pwt = 0;
    page_directory[USER_PAGE_NUM].pcd = 0;
    page_directory[USER_PAGE_NUM].a = 0;
    page_directory[USER_PAGE_NUM].reserved = 0;
    page_directory[USER_PAGE_NUM].ps =1;  // set Page size (PS) flag to be 1
    page_directory[USER_PAGE_NUM].g = 0;
    page_directory[USER_PAGE_NUM].avail = 0;
    page_directory[USER_PAGE_NUM].reserved = 0;
    page_directory[USER_PAGE_NUM].addr = (addr>>PAGING_OFFSET); 
    flush_tlb();
    return;
}

/* 
 * flush_tlb
 * DESCRIPTION: This function is used to flush TLB after swapping page.
 * INPUTS: None
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: update value of eax and cr3
 */
void flush_tlb(){
    asm volatile(
        "movl %%cr3,%%eax   \n"
        "movl %%eax,%%cr3   \n"
        :
        :
        : "memory","cc"
    );
    return;
}

/* 
 * vidmap_paging
 * DESCRIPTION: This function is used to map virtual memory for video mem to physical.
 * INPUTS: None
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: map video memory
 */
void vidmap_paging(){
    page_directory[VIDEO_PAGE_NUM].addr = (unsigned int)page_table_video >> 12;
    page_directory[VIDEO_PAGE_NUM].p = 1;                                    // set to be present
    page_directory[VIDEO_PAGE_NUM].us = 1;                                   // set to user
    page_table_video[0].p = 1;
    if (display_terminal != curr_terminal_running){
        page_table_video[0].addr = VID_MEM + (curr_terminal_running + 1);    // map to backup buffer
    }
    else{
        page_table_video[0].addr = VID_MEM;
    }
    flush_tlb();
    return;
}


