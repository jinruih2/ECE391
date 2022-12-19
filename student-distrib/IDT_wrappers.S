#define ASM 1

#include "x86_desc.h"
#include "multiboot.h"

#define Stack             0x8400000
#define Interrupt_Flag    0x200

.globl systemcall_handler
.globl context_switch


.align 4
/* define the function for 19 exception wrappers */
#define EXCEPTION_WRAPPER(int_handler_num, number)             \
    .globl int_handler_num                                   ;\
    int_handler_num:                                           \
        pushal   /* push all of the registers */              ;\
	    pushfl   /* push all of the flags */          		  ;\
	    pushl $number /* push certain number for halders */   ;\
	    call exception_handler /* call handler */   		  ;\
	    addl $4, %esp  /* remove from stack */                ;\
	    popfl    /* pop all of the flags */               ;\
	    popal    /* pop all of the registers */                   ;\
        iret

/* define the interrupt wrapper for the keyboard */
#define KEYBOARD_INTERRUPT_WRAPPER(handler_name)      \
    .globl handler_name                             ;\
    handler_name:                                     \
        pushal   /* push all of the registers */     ;\
	    pushfl   /* push all of the flags */         ;\
	    call keyboard_int_handler /* call handler */ ;\
	    popfl    /* pop all of the flags */      ;\
	    popal    /* pop all of the registers */   		 ;\
	    iret

/* define the interrupt wrapper for RTC */
#define RTC_INTERRUPT_WRAPPER(handler_name)           \
    .globl handler_name                             ;\
    handler_name:                                     \
        pushal   /* push all of the registers */     ;\
	    pushfl   /* push all of the flags */  		 ;\
	    call rtc_int_handler /* call handler */;\
	    popfl    /* pop all of the flags */      ;\
	    popal    /* pop all of the registers */          ;\
	    iret

/* define the interrupt wrapper for PIT */
#define PIT_INTERRUPT_WRAPPER(handler_name)           \
    .globl handler_name                             ;\
    handler_name:                                     \
        pushal   /* push all of the registers */     ;\
	    pushfl   /* push all of the flags */  		 ;\
	    call pit_int_handler /* call handler */;\
	    popfl    /* pop all of the flags */      ;\
	    popal    /* pop all of the registers */          ;\
	    iret

# a place we set to store the eax value
return_val:
    .long 0

# jump table for 10 system calls
jump_tbl:                   
    .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

# system call handler for 0x80 in IDT
systemcall_handler:
    pushal
    pushfl

    # check the range of jump table, we have 6 system calls 
    cmpl $1,%eax
    jl invalid 
    cmpl $10,%eax
    jg invalid

    pushl %edx
    pushl %ecx
    pushl %ebx

    # to index into the table, we need to decrement eax 
    decl %eax
    sti
    call *jump_tbl(,%eax,4)
    cli

    popl %ebx
    popl %ecx
    popl %edx
    
    # store eax value temporarily
    movl %eax, return_val

    popfl
    popal
    movl return_val, %eax
    iret

    # fail
invalid:
    popfl
    popal
    movl $-1, %eax
    iret

# context_switch for executing a new process
context_switch:
    # load entry point in EBX
    movl 4(%esp),%ebx 

    # push SS               (user-mode stack segment in GDT)
    xorl %eax, %eax
    movw $USER_DS, %ax          
    pushl %eax 

    # push ESP              (where the user program stack is)
    movl $Stack, %eax 
    # -4 to avoid edge 
    addl $-4, %eax      
    pushl %eax

    # push EFLAGS           (EFLAGS for user program)
    pushfl 

    # set IF in eflags, IF is bit 9 in eflags (start at idx 0)
    popl %eax 
    orl $Interrupt_Flag, %eax
    pushl %eax

    # push CS               (user-mode code segment in GDT)
    pushl $USER_CS

    # push EIP              (where the user program starts)
    pushl %ebx

    # switch to user space
iret


/* using the function EXCEPTION_WRAPPER to get 19 handlers for exception */
EXCEPTION_WRAPPER(int_handler_0, 0x0);
EXCEPTION_WRAPPER(int_handler_1, 0x1);
EXCEPTION_WRAPPER(int_handler_2, 0x2);
EXCEPTION_WRAPPER(int_handler_3, 0x3);
EXCEPTION_WRAPPER(int_handler_4, 0x4);
EXCEPTION_WRAPPER(int_handler_5, 0x5);
EXCEPTION_WRAPPER(int_handler_6, 0x6);
EXCEPTION_WRAPPER(int_handler_7, 0x7);
EXCEPTION_WRAPPER(int_handler_8, 0x8);
EXCEPTION_WRAPPER(int_handler_9, 0x9);
EXCEPTION_WRAPPER(int_handler_10, 0xA);
EXCEPTION_WRAPPER(int_handler_11, 0xB);
EXCEPTION_WRAPPER(int_handler_12, 0xC);
EXCEPTION_WRAPPER(int_handler_13, 0xD);
EXCEPTION_WRAPPER(int_handler_14, 0xE);
EXCEPTION_WRAPPER(int_handler_15, 0xF);
EXCEPTION_WRAPPER(int_handler_16, 0x10);
EXCEPTION_WRAPPER(int_handler_17, 0x11);
EXCEPTION_WRAPPER(int_handler_18, 0x12);
EXCEPTION_WRAPPER(int_handler_19, 0x13);

/* using the function KEYBOARD_INTERRUPT_WRAPPER to get the handler for keyboard interrupt */
KEYBOARD_INTERRUPT_WRAPPER(keyboard_handler);

/* using the function RTC_INTERRUPT_WRAPPER to get the handler for RTC interrupt */
RTC_INTERRUPT_WRAPPER(rtc_handler);

/* using the function PIT_INTERRUPT_WRAPPER to get the handler for PIT interrupt */
PIT_INTERRUPT_WRAPPER(pit_handler);



