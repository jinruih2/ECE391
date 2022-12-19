#define ASM 1

#include "x86_desc.h"
#include "multiboot.h"

#define SETFLAG_FOR_CR0 0x80000001
#define SETFLAG_FOR_CR4 0x00000010

.text
.globl enable_paging

.align 4

/* 
 *enable_paging:
 * DESCRIPTION: Enable Paging
 * INPUTS: None
 * OUTPUTS:None
 * RETURN VALUE: None
 * SIDE EFFECTS: C code cannot access computer registers, use assembly to accomplish. Load CR3, CR4, CR0
 */
enable_paging:
    PUSHL %EBP
    MOVL %ESP, %EBP
    PUSHL %EBX
    PUSHL %EDI
    PUSHL %ESI
    MOVL 8(%EBP), %EAX          // move page directory to EAX
    MOVL %EAX, %CR3             // move page directory to CR3
    MOVL %CR4, %EAX
    ORL $SETFLAG_FOR_CR4, %EAX
    MOVL %EAX, %CR4             // enable PSE (4 MiB pages)
    MOVL %CR0, %EAX 
    ORL $SETFLAG_FOR_CR0, %EAX
    MOVL %EAX, %CR0             // set the paging (PG) and protection (PE) bits of CR0
    POPL %ESI
    POPL %EDI
    POPL %EBX
    LEAVE
    ret





