#ifndef _SYSTEMCALL_H
#define _SYSTEMCALL_H

#include "types.h"
#include "rtc.h"
#include "keyboard.h"
#include "paging.h"
#include "terminal.h"

#define MAX_ARGUMENT_SIZE 128   
#define FILE_MIN_NUM 0
#define FILE_MAX_NUM 7
#define STDIN_NUM 0
#define STD_OUT_NUM 1
#define MAX_PID_NUM 6
#define MAX_FNAME_NUM 10
#define MAX_COMMAND_NUM 32
#define VALID_NBYTES 4
#define SUCCESS_RETURN_NBYTES 4
#define START_BYTE_ENTRY_POINT 24
#define NUM_8MB 0x800000
#define NUM_8KB 0x2000
#define NUM_128MB  0x8000000
#define NUM_132MB  0x8400000

typedef struct file_operation_table{
    int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open) (const uint8_t* filename);
    int32_t (*close) (int32_t fd);
}file_operation_table_t;

typedef struct fd{
    file_operation_table_t* file_operation_table;
    int32_t inode;
    int32_t file_pos;       // file position
    int32_t flags;
}fd_t;

typedef struct pcb{
    int8_t pid; 
    int8_t parent_pid;
    fd_t fd_arr[8];         // max file num
    uint32_t saved_esp;     // saved esp for parent esp
    uint32_t saved_ebp;     // saved ebp for parent ebp
    uint32_t return_ebp;    // return ebp for switching terminal
    uint32_t return_esp;    // return esp for switching terminal
    int8_t args[MAX_ARGUMENT_SIZE];
    int active;           
}pcb_t;

// assembly context switch
void context_switch(uint32_t entry);

// system call halt
extern int32_t halt (uint8_t status);

// system call execute
extern int32_t execute (const uint8_t* command);

// system call read
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);

// system call write
extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);

// system call open
extern int32_t open (const uint8_t* filename);

// system call close
extern int32_t close (int32_t fd);

// system call getargs
extern int32_t getargs(uint8_t* buf, int32_t nbytes);

// system call vidmap
extern int32_t vidmap(uint8_t** screen_start);

// system call set_handler
extern int32_t set_handler (int32_t signum, void* handler_address);

// system call sigreturn
extern int32_t sigreturn(void);

// get available pid
int8_t get_pid ();

// initialize the pcb for a process
void init_pcb(pcb_t* pcb, uint32_t pid);

// get the pcb pointer based on pid
pcb_t* get_pcb (uint8_t pid);

// helper function parse command, used in execute
int32_t parse_cmd (const uint8_t* command, uint8_t* fname, int8_t* arg);

// delete a pid when halt
int8_t del_pid(int8_t pid);

// get the current running process pid
pcb_t* get_curr_pcb ();

#endif /* _SYSTEMCALL_H */

