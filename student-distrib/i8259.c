
#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* 
 *i8259_int
 * DESCRIPTION: initialize Intel 8259-A PIC (cascade)
 * INPUTS: None
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: Initialize the Master PIC and Slave PIC
 */
void i8259_init(void) {
    master_mask = 0xff;                             //mask all interrupts on master PIC
    slave_mask = 0xff;                              //mask all interrupts on Slave PIC
    outb(master_mask,MASTER_8259_DATA_PORT);
    outb(slave_mask,SLAVE_8259_DATA_PORT);  

    outb(ICW1,MASTER_8259_COMMAND_PORT);            /*ICW1: select MASTER init*/
    outb(ICW2_MASTER,MASTER_8259_DATA_PORT);        /*ICW2: Master PIC mapped to 0x20 to 0x27*/
    outb(ICW3_MASTER,MASTER_8259_DATA_PORT);        /*ICW3: Master has a Slave PIC on IRQ2*/
    outb(ICW4,MASTER_8259_DATA_PORT);               /*ICW4: eoi*/

    outb(ICW1,SLAVE_8259_COMMAND_PORT);             /*ICW1: select SLAVE init*/
    outb(ICW2_SLAVE,SLAVE_8259_DATA_PORT);          /*ICW2: SLAVE PIC mapped to 0x28 to 0x2F*/
    outb(ICW3_SLAVE,SLAVE_8259_DATA_PORT);          /*ICW3: SLAVE is on IRQ2 of MASTER*/
    outb(ICW4,SLAVE_8259_DATA_PORT);                /*ICW4: eoi*/

    enable_irq(2);                                  //slave is on IRQ2 of MASTER
}

/* 
 *enable_irq
 * DESCRIPTION: enable interrupts for Master PIC and Slave PIC
 * INPUTS: irq_num
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: based on the irq number, unmask interrupt on Master or Slave PIC accordingly
 */
void enable_irq(uint32_t irq_num) {  
    if(irq_num<0 || irq_num>15) return;           
    if(irq_num < 8){                                //Check if the irq comes from master or slave
        master_mask &= ~(1 << irq_num);             //enable the master pic, 1 is 0000 0001 
        outb(master_mask,MASTER_8259_DATA_PORT);
    }
    else{
        irq_num -= 8;                               //Minus 8 to get the corrent offset for to OR'd with slave_mask
        slave_mask &= ~(1 << irq_num);              //enable the slave pic.
        outb(slave_mask,SLAVE_8259_DATA_PORT);
    }
}

/* 
 *disable_irq
 * DESCRIPTION: disable interrupts for Master PIC and Slave PIC
 * INPUTS: irq_num
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: based on the irq number, mask interrupt on Master or Slave PIC accordingly
 */
void disable_irq(uint32_t irq_num) {     
    if(irq_num<0 || irq_num>15) return;        
    if(irq_num < 8){                                //Check if the irq comes from master or slave
        master_mask |= (1 << irq_num);              //disable the master pic, 1 is 0000 0001
        outb(master_mask,MASTER_8259_DATA_PORT);
    }
    else{
        irq_num -= 8;                               //Minus 8 to get the corrent offset for to AND'd with slave_mask
        slave_mask |= (1 << irq_num);               //enable the slave pic.
        outb(slave_mask,SLAVE_8259_DATA_PORT);    
    }
}

/* 
 *send_eoi
 * DESCRIPTION: Inform PIC an interrupt has been handled
 * INPUTS: irq_num
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: based on the irq number, send end-of-interrupt signal to Master or Slave PIC accordingly
 */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8){     
        outb(EOI|(irq_num-8),SLAVE_8259_COMMAND_PORT);  
// check if the irq comes from master or slave, if irq from slave, send eoi to both pics.  
        outb(EOI | 2, MASTER_8259_COMMAND_PORT);        
    }
// If irq from master, it's sufficent to send eoi only to master
    outb(EOI|irq_num,MASTER_8259_COMMAND_PORT);    
}

