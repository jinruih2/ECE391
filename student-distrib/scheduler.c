#include "scheduler.h"

/* void scheduler()
* Input: None
* Output: None
* Return value: None
* Side effect: enable scheduler
*/
void scheduler(){
// get the pid for next process (after switch)    
    int32_t next_pid = get_next_process();  
// only one terminal working, just return
    if (next_terminal_running == curr_terminal_running){
  	send_eoi(0);
  	return;
 }
// remap program (virtual 128MB to Physical)
    map_program(next_pid);
// get current PCB (before switch)
    pcb_t* pcb_before_switch = get_pcb(terminal[curr_terminal_running].curr_pid);
//save esp and ebp
       asm volatile(
            "movl %%esp, %0       \n"
            "movl %%ebp, %1       \n"
            : "=r" (pcb_before_switch->return_esp), "=r" (pcb_before_switch->return_ebp)
            :
            : "memory"
        );   
// get PCB for next process (after switch)
    pcb_t* pcb_after_switch = get_pcb(next_pid);
// get next running terminal
    curr_terminal_running = next_terminal_running;  
// remap video memory
    vidmap_paging();
// prepare for context switch
    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0x800000 - 0x2000 * (next_pid) - 4 ;   // -4 to avoid edge case
	send_eoi(0);
// restore return esp and return ebp 
    asm volatile(
        "movl %0, %%esp       \n"
        "movl %1, %%ebp       \n"
        :
        : "r" (pcb_after_switch->return_esp), "r" (pcb_after_switch->return_ebp)
        : "esp" , "ebp"
    );  
    return;
}

/* void get_next_process()
* Input: None
* Output: None
* Return value: pid for next process
* Side effect: Set correct next_terminal_running
*/
int32_t get_next_process(){
    int i;
    next_terminal_running = curr_terminal_running;
    for(i = 0; i < MAX_TERMINAL; i++){   
        next_terminal_running = (next_terminal_running + 1) % MAX_TERMINAL; 
        if(terminal[next_terminal_running].active == 1){
            break;
        }
    }
    return terminal[next_terminal_running].curr_pid;
}
