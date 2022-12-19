#ifndef INTERRUPT_H
#define INTERRUPT_H
#define ASM 1

#include "x86_desc.h"

//interrupt handlers for 19 exceptions
void int_handler_0(int32_t);
void int_handler_1(int32_t);
void int_handler_2(int32_t);
void int_handler_3(int32_t);
void int_handler_4(int32_t);
void int_handler_5(int32_t);
void int_handler_6(int32_t);
void int_handler_7(int32_t);
void int_handler_8(int32_t);
void int_handler_9(int32_t);
void int_handler_10(int32_t);
void int_handler_11(int32_t);
void int_handler_12(int32_t);
void int_handler_13(int32_t);
void int_handler_14(int32_t);
void int_handler_15(int32_t);
void int_handler_16(int32_t);
void int_handler_17(int32_t);
void int_handler_18(int32_t);
void int_handler_19(int32_t);

//interrupt handler for keyboard
void keyboard_handler(void);
//interrupt handler for RTC
void rtc_handler(void);
//interrupt handler for PIT
void pit_handler(void);
//interrupt handler for system calls
void systemcall_handler(void);

#endif /* INTERRUPT_H */
