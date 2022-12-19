#include "terminal.h"
#include "lib.h"

/* terminal_open
* Description: This function is used to provide access to the file system. 
*              When the named file does not exist, it should return -1. 
* Input: filename -- a named file
* Output: 0 when open call successes
*         -1 when open call fails
* Return value: None
* Side effect: none
*/
int32_t terminal_open(const uint8_t* filename){
    if(filename == NULL){
        return -1;
    }
    return 0;
}

/* terminal_read
* Description: This function read the characters from the terminal and 
                copy the content to the buffer provided.
* Input: fd--file descriptor
         buf--the pointer to the buffer to write into
         nbytes--the size to write
* Output: 0 when open call successes
*         -1 when open call fails
* Return value: None
* Side effect: none
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
     int i = 0;                                            // i is the loop counter
     if(buf == NULL){                                      // ckeck if buffer pointer is valid
        return -1;
    }                                      
   
    char *output = (char*) buf;                            // change the buffer to a char buffer
    while(enter_indict[curr_terminal_running] == 0);
    cli();
    for(i = 0; i <= terminal[curr_terminal_running].kb_idx && i< nbytes; i++){
        output[i] = terminal[curr_terminal_running].kbbuf[i];
        terminal[curr_terminal_running].kbbuf[i] = '\0';
    }
    terminal[curr_terminal_running].kb_idx = 0;
    sti();
    enter_indict[curr_terminal_running] = 0;               // clear the enter indicator
    cli();
    return i;
}

/* terminal_write
* Description: This function takes a buffer, and print the buffer content
                to terminal.
* Input: fd--file descriptor
         buf--the pointer to the buffer to write from
         nbytes--the size to write
* Output: 0 when open call successes
*         -1 when open call fails
* Return value: None
* Side effect: None
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    cli();                                                  // block other interrupt

    if(buf == NULL){                                        // check for valid input
        return -1;
    }

    char *output = (char*)buf;                              // change the buffer to a char buffer

    int i = 0;                                              // i is the loop counter
    for(; i < nbytes; i++){                                 // put the contents in buf to screen
        putc_to_terminal(output[i]);
    }
    sti();                                                  // enable other interrupt
    return nbytes;
}

/* terminal_close
* Description: This function is used to close the file 
* Input: fd--file descriptor
* Output: 0 when open call successes
*         -1 when open call fails
* Return value: None
* Side effect: Close the file
*/
int32_t terminal_close(int32_t fd){
    return 0;
}

/* init_terminal
* Description: This function is used to init 3 terminals
* Input: None
* Output: None
* Return value: None
* Side effect: set memory for each terminals
*/
void init_terminal(){
    uint8_t i;
    uint32_t j;
    for(i = 0; i < TERMINAL_NUM; i++){
        enter_indict[i] = 0;
        terminal[i].tid = i;
        terminal[i].curr_pid = -1;
        terminal[i].active = 0;
        terminal[i].x_pos = 0;
        terminal[i].y_pos = 0;
        terminal[i].kb_idx = 0;
        for(j = 0; j < BUF_SIZE; j++){
            terminal[i].kbbuf[j] = '\0';
        }
        terminal[i].vidmem = (uint8_t*)0xb9000 + i * NUM_4KB;   // backup buffer starts from 0xb9000, see paging.c for more detailed info     
        for (j = 0; j < NUM_COLS * NUM_ROWS; j++) {
			*(uint8_t *)(terminal[i].vidmem + (j << 1)) = ' ';
			//set color attributes for each terminal
        	if (i == 0)     // first terminal
        		*(uint8_t *)(terminal[i].vidmem + (j << 1) + 1) = 0xf;
	    	if (i == 1)     // second terminal
        		*(uint8_t *)(terminal[i].vidmem + (j << 1) + 1) = 0xa;
	   		if (i == 2)     // third terminal
        		*(uint8_t *)(terminal[i].vidmem + (j << 1) + 1) = 0x5;
		}
    }
    // launch terminal 1
    restore_terminal(0);
    display_terminal = 0;
    curr_terminal_running = 0;
    return;
}

/* restore_terminal
* Description: This function is used to restore the terminal we are switching to
* Input: tid -- terminal number (0, 1, 2)
* Output: None
* Return value: return 0; 
* Side effect: update cursor and video memory
*/
int32_t restore_terminal(uint8_t tid){
    set_screen_xy(terminal[tid].x_pos, terminal[tid].y_pos);
    memcpy((uint8_t *)VIDEO, (uint8_t *)terminal[tid].vidmem, 2 * NUM_COLS * NUM_ROWS);    // x2 for color attribute
    return 0; 
}

/* save_terminal
* Description: This function is used to store the terminal we are switching away
* Input: tid -- terminal number (0, 1, 2)
* Output: None
* Return value: return 0; 
* Side effect: update cursor and video memory
*/
int32_t save_terminal(uint8_t tid){
    terminal[tid].x_pos = get_screenx();
    terminal[tid].y_pos = get_screeny();
    memcpy((uint8_t *)terminal[tid].vidmem, (uint8_t *)VIDEO, 2 * NUM_COLS * NUM_ROWS);    // x2 for color attribute
    return 0;
}

/* switch_terminal
* Description: This function is used to switch old terminal to the new terminal
* Input: old_tid , new_tid -- terminal number (0, 1, 2)
* Output: None
* Return value: return 0 for success, return -1 for failure
* Side effect: None
*/
int32_t switch_terminal(uint8_t old_tid, uint8_t new_tid){
    if(save_terminal(old_tid) != 0){
        return -1;
    }
    if(restore_terminal(new_tid) != 0){
        return -1;
    }
    display_terminal = new_tid;
    return 0;
}

/* start_terminal
* Description: This function is used to switch to new terminal when press alt + F1 (or F2, F3)
* Input: tid -- terminal number (0, 1, 2)
* Output: None
* Return value: return 0 for success, return -1 for failure
* Side effect: None
*/
int32_t start_terminal(uint8_t tid){
    cli();
    int ret;
    // sanity check
    if(tid > 2){
        return -1;
    }
    // if we are switching to the current display terminal, just return
    if(tid == display_terminal){
        return 0;
    }
	ret = switch_terminal(display_terminal, tid);
    if(ret != 0){
        return -1;
    }
    // no need to launch shell, just remap video memory
    if(terminal[tid].active == 1){
        vidmap_paging();
	    return 0;
    }else{
    // launch shell and save return esp and ebp
        pcb_t* old_pcb = get_pcb(terminal[curr_terminal_running].curr_pid);        
        asm volatile(
                 "movl %%esp, %0        \n"
                 "movl %%ebp, %1        \n"
                 :"=r"(old_pcb->return_esp), "=r"(old_pcb->return_ebp)    
                 :
                 :"memory"                                          
                 );
	    sti();
        curr_terminal_running = display_terminal;
	    execute((uint8_t*)"shell");
	    return 0; 
    }
}

