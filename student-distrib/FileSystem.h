#ifndef _FILESTSTEM_H
#define _FILESYSTEM_H

#include "types.h"
#include "systemcall.h"

#define filename_len                32
#define reserved_dentry             24
#define reserved_bootblock          52
#define dir_entries_num_bootlock    63
#define inode_maxnum_datablock      1023
#define BLOCK_SIZE                  4096


/*------necessary structs------*/
//struct for directory entry
typedef struct dentry{
    uint8_t filename[filename_len];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[reserved_dentry];
}dentry_t;

// struct for inode
typedef struct inode{
    uint32_t length;
    uint32_t block_num[inode_maxnum_datablock];
}inode_t;

// struct for boot block (first block in filesystem)
typedef struct bootblock{
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_datablocks;
    uint8_t reserved[reserved_bootblock];
    dentry_t dir_entries[dir_entries_num_bootlock];
}bootblock_t;

// struct for data blocks
typedef struct datablock{
    uint8_t data_arr[BLOCK_SIZE];
}datablock_t;

/*------global variables------*/
// pointer to the first block (boot block), indicating the starting address of the file system
bootblock_t* bootblock_ptr;
// current file open
dentry_t curfile;
// current directory 
dentry_t curdirectory;
// current directory index, used by directory read
uint32_t cur_directory_read_idx;

// initialize file system
int32_t filesystem_init(uint32_t starting_addr);

// helper function for file read --- reading the filename
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

// helper function for directory read --- reading the index of directories in bootblock
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

// help function for file read --- read the file contents
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// open a file with given file name
int32_t file_open(const uint8_t* fname);

// close a file
int32_t file_close(int32_t idx);

// write to a file
int32_t file_write();

// read a file
int32_t file_read();

// close a directory
int32_t directory_close();

// write to a directory
int32_t directory_write();

// open a directory
int32_t directory_open();

// read a directory
int32_t directory_read();

// get the size of a file (in bytes)
extern uint32_t get_filelen(uint32_t inode_num);

#endif /* _FILESTSTEM_H */
