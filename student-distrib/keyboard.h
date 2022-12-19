#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "i8259.h"
#include "systemcall.h"
#include "terminal.h"

#define IDT_IRQ1 1
#define DATA_PORT_KEYBOARD_CONTROLLER 0X60
#define CONSOLE_LEN       80
#define CONSOLE_HEIGHT    25
#define VIDEO       0xB8000
  
volatile int enter_indict[3];                                                          // enter indicator; to 1 when enter is pressed to signal for copy operation for terminal_read
int alt_indic;                                                                         // alt indicator. To 1 is alt is pressed. To 0 is alt is released
int shift_indict;                                                                      // shift indicator. To 1 is shift is pressed. To 0 is shift is released
int caps_base;                                                                         // caps base condition, to indicate this is the case for no caps pressed
int caps_cur;                                                                          // caps current condition, compare with caps base to signal if caps is pressed
int ctl_indict;                                                                        // ctl indicator. To 1 is ctl is pressed. To 0 is ctl is released
 
// make the keyboard initialize to IRQ1
void keyboard_init(void);

// handle interrupts for the keyboard
void keyboard_int_handler();

#endif /* _KEYBOARD_H */
