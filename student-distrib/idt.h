#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "IDT_wrappers.h"
#include "systemcall.h"
#define NUM_EXCP  32

// print out the exception messages based on the vector
void exception_handler(int interrupt_idx);

// initialize IDT
void idt_init();

#endif
