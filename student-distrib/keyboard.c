#include "keyboard.h"

//ASCII code for Keyboard inputs
char asccode[57] = {                                                                    // normal looktable when no special key is pressed. 57 is the range for all the char will be used in this check point 
    27 , '1', '2', '3','4', '5', '6', '7', '8', '9', '0', '-', '=', 8,
    9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39, '`', 0,
    92, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0,
    ' '
};
char asccode_shift[57] = {                                                              // shift looktable when shift is pressed. 57 is the range for all the char will be used in this check point
    27 , '!', '@', '#','$', '%', '^', '&', '*', '(', ')', '_', '+', 8,
    9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 13,
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', 34, '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0,
    ' '
};
char asccode_caps[57] = {                                                               // caps looktable when caps is pressed. 57 is the range for all the char will be used in this check point
    27 , '1', '2', '3','4', '5', '6', '7', '8', '9', '0', '-', '=', 8,
    9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 13,
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', 39, '`', 0,
    92, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, 0, 0,
    ' '
};

/* keyboard_init
* Description: This function is used to make the keyboard initialize by 
*              connecting to IRQ1 for keyboard according to IDT.
* Input: None
* Output: None
* Reture value: None
*/
void keyboard_init(void) {
	enable_irq(IDT_IRQ1);
}

/* keyboard_int_handler
* Description: This function is used to handle interrupts for the keyboard. 
*              It irst gets the data from keyboard data port 0x60 and then 
*              if it has ASCII code only let lower case characters and number 
*              on keyboard shown on screen.
* Input: None
* Output: None
* Reture value: None
*/
void keyboard_int_handler() {
    uint8_t keyboard_asccode, keyboard_scancode;
	 cli();                                                      // mask interrupt

	keyboard_scancode = inb(DATA_PORT_KEYBOARD_CONTROLLER);      // keyboard scancode from keyboard data port
	if(keyboard_scancode == 0x38){                               // 0x38 is the scancode for alt pressed
        alt_indic = 1;                                           // set alt indicator to 1
        send_eoi(IDT_IRQ1);
        sti();
        return; 
    }
     if(keyboard_scancode == 0xB8){                              // 0xB8 is the scancode of alt released
        alt_indic = 0;                                           // set alt indicator tp 0
        send_eoi(IDT_IRQ1);
        sti();
        return; 
    }
    if(keyboard_scancode == 0x3B){                               // 0x3B is the scancode for F1
        if(alt_indic == 1){                                      // if alt is pressed, shift to terminal 1
            send_eoi(IDT_IRQ1);
            sti();
            start_terminal(0);                                   // open the first terminal
            display_terminal = 0;                                // set display_terminal to 0
            return;
        }
        send_eoi(IDT_IRQ1);
        sti();
        return; 
    }
    if(keyboard_scancode == 0x3C){                               // 0x3C is the scancode of F2
        if(alt_indic == 1){                                      // if alt is pressed, shift to terminal 2
             send_eoi(IDT_IRQ1);
            sti();
            start_terminal(1);                                   // open the second terminal
            display_terminal = 1;                                // set display_terminal to 1
            return;
        }
        send_eoi(IDT_IRQ1);
        sti();
        return;
    }
    if(keyboard_scancode == 0x3D){                               // 0x3C is the scancode of F3
        if(alt_indic == 1){                                      // if alt is pressed, shift to terminal 2
            send_eoi(IDT_IRQ1);
            sti();
            start_terminal(2);                                   // open the third terminal
            display_terminal = 2;                                // set display_terminal to 2
            return;
        }
        send_eoi(IDT_IRQ1);
        sti();
        return;
    }
    if(keyboard_scancode == 0x2A || keyboard_scancode == 0x36){  // check if shift is pressed
                                                                 // 0x2A is the scane code for lshift; 0x36 is rshift
        shift_indict = 1;                                        // set the shift indicator   
    }
    if(keyboard_scancode == 0xAA || keyboard_scancode == 0XB6){  // check if shif is released
                                                                 // lshift release is 0xAA; rshift release is 0xB6
        shift_indict = 0;

    }


    if(keyboard_scancode == 0x3A){                               // check if caps is pressed
        if(caps_base == caps_cur){                               // caps scane code is 0x3A
            caps_cur++;
        }
        else{
            caps_cur--;
        }
    }


    if(keyboard_scancode == 0x1D){                               // check if ctl is pressed
        ctl_indict = 1;                                          // 0x1D is the scane code for ctl
    }
    if(keyboard_scancode == 0x9D){                               // check if ctl is released
        ctl_indict = 0;                                          // 0x9D is the ctl release scane code
    }


	if (keyboard_scancode <= 57){                                // 57 is the range for regular characters
        if(keyboard_scancode == 0x1c){                           // check if "new_line" is pressed, 0x1c is the scane code for '\n'
            putc_modified('\n');        
            terminal[display_terminal].kbbuf[   terminal[display_terminal].kb_idx] = '\n';                    
            enter_indict[display_terminal] = 1;                  // set the enter indicator
        }
        else if(keyboard_scancode == 0x0E){                      // check if backspace is pressed, 0x0E is the scane code for back space
            if(terminal[display_terminal].kbbuf[0] != '\0'){     // if kb_buf is empty, do not delete
                terminal[display_terminal].kb_idx--;
                if(terminal[display_terminal].kbbuf[   terminal[display_terminal].kb_idx] == '\t'){              // if the next char is tab, invike the tab delete
                    delete_tab();
                }
                else{                                            // else, invoke normal delete
                    deletc();
                }
                terminal[display_terminal].kbbuf[   terminal[display_terminal].kb_idx] = '\0';                   // clear the last space in kb buffer
           }
        }
        else if(ctl_indict == 1 && keyboard_scancode == 0x26){   // check if clear screen
            clear_helper();
        }

        else{
            if(shift_indict == 1){                                      // if shift is pressed, use the shift version looktable
                keyboard_asccode = asccode_shift[keyboard_scancode-1];  
                if(keyboard_asccode == 0){                              // if kb_ascode is 0, means we should not need to handle the case for now, eoi and return
                    send_eoi(IDT_IRQ1);
	                sti();
                    return;
                }
                if( terminal[display_terminal].kb_idx <= BUF_SIZE - 2){ // check if the buffer is filled
                    putc_modified(keyboard_asccode);                    // print the char
                    terminal[display_terminal].kbbuf[   terminal[display_terminal].kb_idx] = keyboard_asccode;       // fill in the kb_buffer
                    terminal[display_terminal].kb_idx++;                // kb indicator is increased indicating that a key is pressed
                }
            }
            else if(shift_indict == 0 ){                                // if shift is not pressed
                if(caps_cur == caps_base){                              // if caps is not pressed, use the normal version of looktable
                    keyboard_asccode = asccode[keyboard_scancode-1];
                    if(keyboard_asccode == 0){                          // if kb_ascode is 0, means we should not need to handle the case for now, eoi and return
                        send_eoi(IDT_IRQ1);
	                    sti();
                        return;
                    }
                    if(terminal[display_terminal].kb_idx <= BUF_SIZE - 2){                                           // check if the buffer is filled
                        putc_modified(keyboard_asccode);                // print the char
                        terminal[display_terminal].kbbuf[   terminal[display_terminal].kb_idx] = keyboard_asccode;   // fill in the kb_buffer
                        terminal[display_terminal].kb_idx++;            // kb indicator is increased indicating that a key is pressed
                    }
                }
                else{                                                   // if caps is pressed, use the caps version looktable
                    keyboard_asccode = asccode_caps[keyboard_scancode-1];
                     if(keyboard_asccode == 0){                         // if kb_ascode is 0, means we should not need to handle the case for now, eoi and return
                        send_eoi(IDT_IRQ1);
	                    sti();
                        return;
                    }
                    if(terminal[display_terminal].kb_idx <= BUF_SIZE - 2){                       // check if the buffer is filled
                        putc_modified(keyboard_asccode);                                         // print the char
                        terminal[display_terminal].kbbuf[   terminal[display_terminal].kb_idx] = keyboard_asccode;   // fill in the kb_buffer
                        terminal[display_terminal].kb_idx++;                                     // kb indicator is increased indicating that a key is pressed
                    }
                }
            }
        }
	}
	send_eoi(IDT_IRQ1);
	sti();                                                      // enable interrupt
}

