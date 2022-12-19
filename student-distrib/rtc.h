#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"

/*
Define the port used by RTC
*/
#define RTC_PORT 0X70
#define CMOS_PORT 0x71

/*
Define the register used by RTC
*/
#define REGISTER_A 0X0A
#define REGISTER_B 0X0B
#define REGISTER_C 0X0C

/*
Define the IDT number for RTC interrupt
*/
#define RTC_PIC_NUM 8    


#define INTERRUPT_DEFAULT_RATE 2
#define RTC_ACCEPT_INT 4
#define MAX_FREQUENCY 32768
#define MASK_TOP_FOUR 0xF0
#define NUM_RATE 10
#define NUM_TERMINAL 3

// initialize RTC
void rtc_init();
// set RTC interrupt handler
void rtc_int_handler();

// provide access to the file system
int32_t rtc_open(const uint8_t * filename);

// close certain file descriptor and let it available for later open calls
int32_t rtc_close(int32_t fd);

// read data from the device RTC
int32_t rtc_read(int32_t fd, void * buf, int32_t nbytes);

// write data to the device RTC
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes);

// change the interrupt rate according to new frequency value
int32_t set_RTC_interrupt_rate(int32_t set_frequency);

// change the rate
void change_interrupt_rate(unsigned int rate);

#endif /*_RTC_H*/

