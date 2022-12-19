#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "enable_paging.h"
#include "terminal.h"
#define PAGE_SIZE 1024
#define FOUR_K    4096
#define ONE_K     1024
#define NUMBER_8MB 0x800000
#define NUMBER_4MB 0x400000
#define PAGING_OFFSET 12
#define USER_PAGE_NUM 32
#define VIDEO_PAGE_NUM 33
#define VID_MEM   0xB8
// intel manual 3-24 Figure 3-14. Format of Page-Directory and Page-Table Entries for 4-KByte Pages
// and 32-Bit Physical Addresses
// page directory struct
typedef union page_directory{
    uint32_t val[1];
    struct{
        uint32_t p:1;   // Present (P) flag, bit 0. Indicates whether the page or page 
                        // table being pointed to by the entry is currently loaded in physical memory. 
        uint32_t rw:1;  // Read/write (R/W) flag, bit 1
        uint32_t us:1;  // User/supervisor (U/S) flag, bit 2
        uint32_t pwt:1; // Page-level write-through (PWT) flag, bit 3
        uint32_t pcd:1; // Page-level cache disable (PCD) flag, bit 4
        uint32_t a:1;   // Accessed (A) flag, bit 5. Indicates whether a page or page table has been accessed.
        uint32_t reserved:1; 
        uint32_t ps:1;  // Page size (PS) flag, bit 7 page-directory entries for 4-KByte pages.
                        // Determines the page size. When this flag is clear, the page size is 4 KBytes and
                        // the page-directory entry points to a page table. When the flag is set, the page
                        // size is 4 MBytes for normal 32-bit addressing and the page-directory entry points to a page. 
        uint32_t g:1;   // Global (G) flag, bit 8.  Indicates a global page when set.
        uint32_t avail:3;   // available-to-software bits
        uint32_t addr:20;
    } __attribute__ ((packed));
}page_directory_t;

// page table struct
typedef union page_table{
    uint32_t val[1];
    struct{
        uint32_t p:1;
        uint32_t rw:1;
        uint32_t us:1;
        uint32_t pwt:1;
        uint32_t pcd:1;
        uint32_t a:1;
        uint32_t d:1;   // Dirty (D) flag, bit 6. Indicates whether a page has been written to when set.
        uint32_t pat:1;
        uint32_t g:1;
        uint32_t avail:3;
        uint32_t addr:20;
    } __attribute__ ((packed));
}page_table_t;

// initialize paging
void paging_init();

// load and map the program
void map_program(uint32_t pid);

// flush tlb after swapping page
void flush_tlb();

// video paging map
void vidmap_paging();

#endif

