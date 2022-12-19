#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "lib.h"
#include "systemcall.h"
#define TERMINAL_NUM    3
#define NUM_4KB         0x1000
//terminal open with filename
int32_t terminal_open(const uint8_t* filename);

//terminal read from keyboard and copy to the buffer
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

//terminal write from the buffer to console
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

//terminal close
int32_t terminal_close(int32_t fd);

// initialize terminals
void init_terminal();

// restore the terminal we are switching to
int32_t restore_terminal(uint8_t tid);

// store terminal we are switching away
int32_t save_terminal(uint8_t tid);

// helper function swtich terminals
int32_t switch_terminal(uint8_t old_tid, uint8_t new_tid);

// used when press alt + F1 (or F2, F3), to switch to given tid
int32_t start_terminal(uint8_t tid);

typedef struct terminal{
    uint8_t tid;                        // terminal id (0,1,2)
    int8_t curr_pid;                    // current running process on terminal
    volatile uint8_t kbbuf[BUF_SIZE];   // keyboard buffer
    volatile uint8_t kb_idx;            // keyboard index
    uint8_t* vidmem;                    // video memory location
    uint8_t active;                     // check if shell is running
    uint32_t x_pos;                     // cursor x coordinate
    uint32_t y_pos;                     // cursor y coordinate
}terminal_t;

terminal_t terminal[TERMINAL_NUM];

// to indicate the current running terminal
volatile uint8_t curr_terminal_running;
// to indicate the current display terminal
volatile uint8_t display_terminal;

#endif /* _TERMINAL_H */
