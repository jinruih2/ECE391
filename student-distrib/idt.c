
#include "idt.h"

/* 
 *exception_handler
 * DESCRIPTION: print out exception messages for 19 exceptions
 * INPUTS: interrupt index for exceptions
 * OUTPUTS:exception messages 
 * RETURN VALUE: None
 * SIDE EFFECTS: exception messages displayed on the screen
 */
void exception_handler(int interrupt_idx){
    switch(interrupt_idx){
        case 0: printf("Divide-by-zero Error\n");
            while(1);
            break;
        case 1: printf("Debug\n");
            while(1);
            break;
        case 2: printf("Non-maskable Interrupt\n");
            while(1);
            break;
        case 3: printf("Breakpoint\n");
            while(1);
            break;
        case 4: printf("Overflow\n");
            while(1);
            break;
        case 5: printf("Bound Range Exceeded\n");
            while(1);
            break;
        case 6: printf("Invalid Opcode\n");
            while(1);
            break;
        case 7: printf("Device Not Available\n");
            while(1);
            break;
        case 8: printf("Double Fault\n");
            while(1);
            break;
        case 9: printf("Coprocessor Segment Overrun\n");
            while(1);
            break;
        case 10: printf("Invalid TSS\n");
            while(1);
            break;
        case 11: printf("Segment Not Present\n");
            while(1);
            break;
        case 12: printf("Stack-Segment Fault\n");
            while(1);
            break;
        case 13: printf("General Protection Fault\n");
            while(1);
            break;
        case 14: printf("Page Fault\n");
            while(1);
            break;
        case 16: printf("x87 Floating-Point Exception\n");
            while(1);
            break;
        case 17: printf("Alignment Check\n");
            while(1);
            break;
        case 18: printf("Machine Check\n");
            while(1);
            break;
        case 19: printf("SIMD Floating-Point Exception\n");
            while(1);
            break;
        default: printf("Unknown Exception\n");
            while(1);
    }
}

/* 
 *idt_int
 * DESCRIPTION: Initialize IDT using Interrupt Gate 01100
 * INPUTS: None
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: IDT has been initialized
 */
void idt_init(){
    int i;
    for(i = 0; i < NUM_VEC; i++){
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0x0;
        idt[i].reserved3 = 0x0;
        idt[i].reserved2 = 0x1;
        idt[i].reserved1 = 0x1;
        idt[i].size = 0x1;
        idt[i].reserved0 = 0x0;
        idt[i].dpl = 0x0;
        idt[i].present = 0x1;

        if(i < NUM_EXCP){                                 
            idt[i].reserved3 = 0x1;
        }
        
        if(i == 0x80){                              // set dpl = 3 so that user program has the access
            idt[i].dpl = 0x3;
        }
    }

// set exception entries in the IDT table
    SET_IDT_ENTRY(idt[0], int_handler_0);
    SET_IDT_ENTRY(idt[1], int_handler_1);
    SET_IDT_ENTRY(idt[2], int_handler_2);
    SET_IDT_ENTRY(idt[3], int_handler_3);
    SET_IDT_ENTRY(idt[4], int_handler_4);
    SET_IDT_ENTRY(idt[5], int_handler_5);
    SET_IDT_ENTRY(idt[6], int_handler_6);
    SET_IDT_ENTRY(idt[7], int_handler_7);
    SET_IDT_ENTRY(idt[8], int_handler_8);
    SET_IDT_ENTRY(idt[9], int_handler_9);
    SET_IDT_ENTRY(idt[10], int_handler_10);
    SET_IDT_ENTRY(idt[11], int_handler_11);
    SET_IDT_ENTRY(idt[12], int_handler_12);
    SET_IDT_ENTRY(idt[13], int_handler_13);
    SET_IDT_ENTRY(idt[14], int_handler_14);
    SET_IDT_ENTRY(idt[15], int_handler_15);
    SET_IDT_ENTRY(idt[16], int_handler_16);
    SET_IDT_ENTRY(idt[17], int_handler_17);
    SET_IDT_ENTRY(idt[18], int_handler_18);
    SET_IDT_ENTRY(idt[19], int_handler_19);

// set PIT Interrupt entry in the IDT table
    SET_IDT_ENTRY(idt[0x20], pit_handler);
// set Keyboard Interrupt entry in the IDT table
    SET_IDT_ENTRY(idt[0x21], keyboard_handler);
// set RTC Interrupt entry in the IDT table
    SET_IDT_ENTRY(idt[0x28], rtc_handler);
// set System Call entry in the IDT table
    SET_IDT_ENTRY(idt[0x80], systemcall_handler);
}
