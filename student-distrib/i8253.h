#ifndef _I8253_H
#define _I8253_H

#include "lib.h"
#include "types.h"
#include "scheduler.h"
#include "i8259.h"

#define FREQUENCY 			    1193182
#define CHANNEL_0 				0x40
#define CMD_REG					0x43
#define MASK                    0Xff
#define RIGHT_SHIFT_8           8
#define DIVIDE_COUNTER          100

// initialize PIT
void i8253_init();

#endif /* _I8253_H */
