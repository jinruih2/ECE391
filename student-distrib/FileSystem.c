#include "FileSystem.h"
#include "lib.h"
#include "types.h"
#include "systemcall.h"
/* 
 *init_filesystem
 * DESCRIPTION: initialize the file system
 * INPUTS: starting_addr
 * OUTPUTS:None
 * RETURN VALUE: return 0 for success, return -1 for failure
 * SIDE EFFECTS: Set the pointer pointing to the bootblock
 */
int32_t filesystem_init(uint32_t starting_addr){
    if(starting_addr == NULL){                               //check the validity of the address
        return -1;
    }
    else{
        bootblock_ptr = (bootblock_t*)starting_addr;         //valid address
        return 0;
    }
}

/* 
 *read_dentry_by_name
 * DESCRIPTION: fill the directory entry with given filename
 * INPUTS: fname, dentry
 * OUTPUTS:None
 * RETURN VALUE: return 0 for success, return -1 for failure
 * SIDE EFFECTS: the entry is filled
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    if(strlen((int8_t*)fname)>filename_len){
        return -1;
    }
    //non-exist file, return -1
    int i,j;
    for(i=0;i<bootblock_ptr->num_dir_entries;i++){                       //go through each directory entry
        int file_found =1;                                               //set the found flag
        dentry_t cur_dentry = bootblock_ptr->dir_entries[i];
        for(j=0;j<filename_len;j++){                                     //go through each character in filename
            if(fname[j]!=cur_dentry.filename[j]){
                file_found = 0;
                break;
            }
            else{
                if(cur_dentry.filename[j] == 0 || fname[j] ==0) break;   //reach the end of file
            }
                
        }
        if(file_found){
            curfile = cur_dentry;                                        //store it as a global variable
            for(j=0;j<filename_len;j++){
                dentry->filename[j] = cur_dentry.filename[j];
            }
            dentry->filetype = cur_dentry.filetype;
            dentry->inode_num = cur_dentry.inode_num;                    //copy information to dentry
            return 0;
        }
    }
    //fill in dentry with the file name, file type, inode number of the file
    return -1;
}

/* 
 *read_dentry_by_index
 * DESCRIPTION: fill the directory entry with given index
 * INPUTS: index, dentry
 * OUTPUTS:None
 * RETURN VALUE: return 0 for success, return -1 for failure
 * SIDE EFFECTS: the entry is filled
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    //invalid index, return -1
    if(index<0 || index>=bootblock_ptr->num_dir_entries){
        return -1;
    } 
    int i;
    curdirectory = bootblock_ptr->dir_entries[index];                     //go to the directory of given index
    for(i=0;i<filename_len;i++){                                         
        dentry->filename[i]=curdirectory.filename[i];
    }
    dentry->filetype = curdirectory.filetype;
    dentry->inode_num = curdirectory.inode_num;                           //copy info into dentry
    return 0;
}

/* 
 *read_data
 * DESCRIPTION: read bytes (begin at the offset) in the file (indicated by inode) and return the number of bytes read and placed in the buffer
 * INPUTS: inode, offset, buf, length
 * OUTPUTS:None
 * RETURN VALUE: return the number of bytes read for success, return -1 for failure
 * SIDE EFFECTS: more bytes are read
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    inode_t* inode_ptr;
    uint32_t block;
    uint32_t num_block;
    uint8_t* start_addr;
    uint8_t* cur_buf = buf;
    uint32_t rem_length = length;
    datablock_t* datablock_addr;
    datablock_t* start_block;
    uint32_t read_length;
    if(inode >= bootblock_ptr->num_inodes){
        return -1;  //invalid inode number, return -1
    }
    if(!buf){
        return -1;  // invalid buf
    }
    inode_ptr = (inode_t*)((void*)bootblock_ptr + BLOCK_SIZE) + inode;
    if((inode_ptr -> length - offset) < length){
        read_length = inode_ptr -> length - offset;
    }else{
        read_length = length;
    }
    //read up to length(3rd argument) bytes starting from position offset(2nd argument)
    block = offset / BLOCK_SIZE;    // get the start data block position
    if(inode_ptr->length % BLOCK_SIZE){ // get the number of block info from inode
        num_block = (inode_ptr->length / BLOCK_SIZE) + 1;
    }else{
        num_block = (inode_ptr->length / BLOCK_SIZE);
    }
    if(block >= num_block){ // check data block bound
        return -1;
    }
    datablock_addr = (datablock_t*)((inode_t*)((void*)bootblock_ptr + BLOCK_SIZE) + bootblock_ptr->num_inodes);
    start_block = datablock_addr + inode_ptr->block_num[block];
    start_addr = (uint8_t*)start_block + offset % BLOCK_SIZE;
    // get the start read position
    if(read_length <= (BLOCK_SIZE - offset % BLOCK_SIZE)){    // only read the start block
        memcpy(cur_buf, (uint8_t*)start_addr, read_length);
        return read_length;
    }else{
        memcpy(cur_buf, (uint8_t*)start_addr, BLOCK_SIZE - offset % BLOCK_SIZE);  // read the start block
        rem_length = read_length - BLOCK_SIZE + (offset % BLOCK_SIZE);  // update the remaining length to read
        cur_buf = cur_buf + BLOCK_SIZE - offset % BLOCK_SIZE;   // update the position to store in the buf
        block++;    // update the data block position
    }
    while (rem_length > 0){    // read until there is no remaining length to read 
        if(block >= num_block){ // check block bound
            return cur_buf - buf;
        }
        start_block = datablock_addr + inode_ptr->block_num[block];
        start_addr = (uint8_t*)start_block;
        // update start read position
        if(rem_length < BLOCK_SIZE){   // last block to be read
            memcpy(cur_buf, (uint8_t*)start_addr, rem_length);
            return cur_buf - buf + rem_length;
        }else{  // read the entire block and then update the remaining length, buf position, and block info
            memcpy(cur_buf, (uint8_t*)start_addr, BLOCK_SIZE);
            rem_length = rem_length - BLOCK_SIZE;
            cur_buf = cur_buf + BLOCK_SIZE;
            block++;
        }
    }
    return cur_buf - buf;   // return length read
}

/* 
 *directory_open
 * DESCRIPTION: opens a directory file
 * INPUTS: fname
 * OUTPUTS:None
 * RETURN VALUE: return 0 for success, return -1 for failure
 * SIDE EFFECTS: do nothing because we have a read-only file system
 */
int32_t directory_open(const uint8_t* fname){
   int32_t result = read_dentry_by_name(fname,&curdirectory);       // check the validity of filename
   if(result<0){
    return -1;
   }
   if(curdirectory.filetype==1){                                    // 1 means the directory type number
    cur_directory_read_idx = 0;                                     // set the index to 0
    return 0;
   }
   return -1;
}

/* 
 *directory_close
 * DESCRIPTION: close the file
 * INPUTS: fd
 * OUTPUTS:None
 * RETURN VALUE: return 0 
 * SIDE EFFECTS: do nothing
 */
int32_t directory_close(int32_t fd){
    return 0;
}

/* 
 *directory_read
 * DESCRIPTION: read a directory file
 * INPUTS: fd, buf, nbytes
 * OUTPUTS:None
 * RETURN VALUE: return 0 for success, return -1 for failure
 * SIDE EFFECTS: do nothing because we have a read-only file system
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    if(!buf) return -1;                                                            // invalid buffer
    if(cur_directory_read_idx==bootblock_ptr->num_dir_entries) return 0;           // at the last directory entry, just return
    int32_t result = read_dentry_by_index(cur_directory_read_idx,&curdirectory);   // check index validity
    if(result<0) return -1;                                                        // failed reading index
    cur_directory_read_idx ++;
    memcpy(buf,curdirectory.filename,filename_len);                                // copy the filename to buffer
    return filename_len;
}

/* 
 *directory_write
 * DESCRIPTION: NONE
 * INPUTS: fd, buf, nbytes
 * OUTPUTS:None
 * RETURN VALUE: return -1
 * SIDE EFFECTS: do nothing because we have a read-only file system
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/* 
 *file_open
 * DESCRIPTION: open the file (if file name is valid)
 * INPUTS: fname
 * OUTPUTS:None
 * RETURN VALUE: return 0 for success, return -1 for failure
 * SIDE EFFECTS: if the filename exists, just return (for checkpoint 6.2)
 */
int32_t file_open(const uint8_t* fname){
    uint32_t result = read_dentry_by_name(fname, &curfile);     //check validity of filename
    if(result < 0) return -1;
    return 0;
}

/* 
 *file_close
 * DESCRIPTION: close the open file
 * INPUTS: fd
 * OUTPUTS:None
 * RETURN VALUE: return 0 
 * SIDE EFFECTS: do nothing (for checkpoint 6.2)
 */
int32_t file_close(int32_t fd){
    return 0;
}

/* 
 *file_read
 * DESCRIPTION: read the file
 * INPUTS: fd, buf, nbytes
 * OUTPUTS:None
 * RETURN VALUE: return -1 for failure, return the number of bytes read for success
 * SIDE EFFECTS: return the number of bytes read
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    if(!buf) return -1;                                                                                       // invalid buffer
    
    pcb_t* curr_pcb = get_curr_pcb();                                                                         // get the current pcb pointer
    int32_t result = read_data(curr_pcb -> fd_arr[fd].inode, curr_pcb -> fd_arr[fd].file_pos, buf, nbytes);   // check if we can read
    if(result < 0) return -1;                                                                                 // fail reading
    curr_pcb -> fd_arr[fd].file_pos += result;                                                                // update the read position in pcb
    return result;  
}

/* 
 *file_write
 * DESCRIPTION: write to the file
 * INPUTS: idx, buf, nbytes
 * OUTPUTS:None
 * RETURN VALUE: return -1
 * SIDE EFFECTS: nothing because we are using the read-only filesystem
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/* 
 *get_filelen
 * DESCRIPTION: get the length of file
 * INPUTS: inode_num -- number of inode
 * OUTPUTS: None
 * RETURN VALUE: value of length of file in inode
 * SIDE EFFECTS: get inode_ptr
 */
uint32_t get_filelen(uint32_t inode_num){
    // get the inode pointer
    inode_t* inode_ptr = (inode_t*)((void*)bootblock_ptr + BLOCK_SIZE) + inode_num;
    return inode_ptr -> length;
}
