#include "i8253.h"

/* 
 *i8253_int
 * DESCRIPTION: initialize Intel 8253 Programmable Interval Timer (PIT)
 * INPUTS: None
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: Initialize the PIT
 */
void i8253_init(){
    int32_t freq = FREQUENCY / DIVIDE_COUNTER; // interrupt every 10ms
    int32_t freq_low  = freq &  MASK;                 
    int32_t freq_high = freq >> RIGHT_SHIFT_8;       

    // send command to the port     
    // bit 6 and 7 (select channel):  0 0 -> channel 0 
    // bit 4 and 5 (Access mode):     1 1 -> lobyte/hibyte 
    // bit 1 to  3 (operating mode):  0 1 0 -> Mode 2, rate generator 
    // bit 0      (BCD/Binary mode):  0 -> 16-bit binary 
    // That is why we have 0x34 ->    0011 0100 
    outb(0x34, CMD_REG);  
    outb(freq_low, CHANNEL_0);
    outb(freq_high,CHANNEL_0);
    // the output for PIT channel 0 is connected to the PIC IRQ 0
    enable_irq(0);                
    return;
}

void pit_int_handler(){
    send_eoi(0);
    cli();
    // implement scheduler only when terminal 2 or terminal 3 is running 
    if(terminal[1].active == 1 || terminal[2].active == 1){
        scheduler();
    }
    sti();
    return;
}

