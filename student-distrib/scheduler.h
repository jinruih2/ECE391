#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "systemcall.h"
#include "i8259.h"
#include "paging.h"
#include "keyboard.h"

#define MAX_TERMINAL 3

uint32_t next_terminal_running;

// implement Scheduler
void scheduler();

// get the pid for next running process
int32_t get_next_process();

#endif /* _SCHEDULER_H */
