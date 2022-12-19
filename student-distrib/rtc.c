// Reference: https://wiki.osdev.org/RTC

#include "rtc.h"
#include "lib.h"
#include "types.h"
#include "scheduler.h"

// set interrupt flags for each of three terminals
volatile int interrupt_flags[NUM_TERMINAL] = {1, 1, 1};

/* rtc_init
* Description: This function is used to init the device RTC.
* Input: None
* Output: None
* Return value: None
* Side effect: Turning on IRQ8
*/
void rtc_init(void){                            
    outb(REGISTER_A,RTC_PORT);                  // Choose register b, and disable NMI
    unsigned char prev = inb(CMOS_PORT);        // Read current value to register b
    outb(REGISTER_B,RTC_PORT);                  // Set index again
    outb(prev | 0x40,CMOS_PORT);                // write the previous value and or with 0x40. This turns on bit 6 of register B  
    enable_irq(RTC_PIC_NUM);                    // Enable interrupt request from PIC                           
}

/* rtc_interrupt
* Description: This function is used to the RTC interrupt handler when called by the IDT .
* Input: None
* Output: None
* Return value: None
* Side effect: Clear the flag of indicating interrupt
*/
void rtc_int_handler(){
    cli();
    outb(REGISTER_C,RTC_PORT);             //mask the other interrupt
    inb(CMOS_PORT);                         //Throw away contents
    send_eoi(RTC_PIC_NUM);
    // for three terminals
    int num;
    for (num = 0; num < NUM_TERMINAL; num++) {
        interrupt_flags[num] = 0; // interrupt handler clears the interrupt flags
    }
    sti();
}

/* rtc_open
* Description: This function is used to provide access to the file system. 
*              When the named file does not exist, it should return -1. 
* Input: filename -- a named file
* Output: 0 when open call successes
*         -1 when open call fails
* Return value: None
* Side effect: set default frequency 2 Hz to rate
*/
int32_t rtc_open(const uint8_t * filename) {
    if (filename == NULL) { // check whether named file is null or not
        return -1;
    } else {
        return set_RTC_interrupt_rate(INTERRUPT_DEFAULT_RATE); // not null set default rate 2
    }
}

/* rtc_close
* Description: This function is used to close certain file descriptor and 
*              let it available for later open calls. 
* Input: fd -- file descriptor
* Output: always 0 for close success
* Return value: None
* Side effect: close file descriptor
*/
int32_t rtc_close(int32_t fd) {
    return 0; // should return 0 always
}

/* rtc_read
* Description: This function is used to read data from the device RTC. 
* Input: fd -- a file descriptor
*        buf -- a buffer provided with the value of new interrupt rate
*        nbytes -- the number of bytes written
* Output: always 0
* Return value: None
* Side effect: clear the interrupt flag after interrupt happens
*/
int32_t rtc_read(int32_t fd, void * buf, int32_t nbytes) {
    // set interrupt flag for current running terminal in order for happened interrupt
    interrupt_flags[curr_terminal_running] = 1;
    while (interrupt_flags[curr_terminal_running] == 1) {
        ; // wait until the interrupt handler clears the interrupt flag
    }
    interrupt_flags[curr_terminal_running] = 1; // set the flag current running terminal again for checking next interrupt 
    return 0; // should return 0 always
}

/* rtc_write
* Description: This function is used to write data to the device RTC.
* Input: fd -- a file descriptor
*        buf -- a buffer provided with the value of new interrupt rate
*        nbytes -- the number of bytes written
* Output: the number of bytes written when write call successes
*         -1 when write call fails
* Return value: None
* Side effect: set the rate of periodic interrupt
*/
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes) {
    /* 
        fails when there are no interrupt rate value in buffer or 
        integer specifying the interrupt rate in Hz is not 4-byte
    */
    if (buf == NULL || nbytes != RTC_ACCEPT_INT) {
        return -1; 
    }
    int frequency_interrupt = * (int *) buf; // get the value of new interrupt rate in buffer
    int32_t s = set_RTC_interrupt_rate(frequency_interrupt); //  set the rate of periodic interrupt
    if (s == 0) { // equal to 0 then set interrupt rate succusses
        return nbytes; 
    } else {
        return -1; // set interrupt rate fails
    }
}

/* set_RTC_interrupt_rate
* Description: This function is used to change the interrupt rate according to new frequency value.
* Input: set_frequency -- new frequency value
* Output: 0 when RTC could generate this value of frequency
*         -1 when RTC could not generate this value of frequency
* Return value: None
* Side effect: change the interrupt rate
*/
int32_t set_RTC_interrupt_rate(int32_t set_frequency) {
    /* 
        RTC could only generate frequency with value of a power of two. And the maximum frequency 
        is 1024 Hz, and the minimum frequency is the default frequency 2 Hz set in rtc_open. From 
        1024 to 2, there are 10 number whose value is a power of two.
    */
    int32_t valid_frequency[NUM_RATE] = {1024, 512, 256, 128, 64, 32, 16, 8, 4, 2};
    /* 
        We could use the formula frequency =  32768 >> (rate - 1) to get the rate. 32768 = 0x8000.
        0x8000 shift right 5 bits to have 1024 = 0x0400. rate-1 = 5 -> rate=6.
        0x8000 shift right 6 bits to have 512 = 0x0200. rate-1 = 6 -> rate=7.
        0x8000 shift right 7 bits to have 256 = 0x0100. rate-1 = 7 -> rate=8.
        0x8000 shift right 8 bits to have 128 = 0x0080. rate-1 = 8 -> rate=9.
        0x8000 shift right 9 bits to have 64 = 0x0040. rate-1 = 9 -> rate=10.
        0x8000 shift right 10 bits to have 32 = 0x0020. rate-1 = 10 -> rate=11.
        0x8000 shift right 11 bits to have 16 = 0x0010. rate-1 = 11 -> rate=12.
        0x8000 shift right 12 bits to have 8 = 0x0004. rate-1 = 12 -> rate=13.
        0x8000 shift right 13 bits to have 4 = 0x0002. rate-1 = 13 -> rate=14.
        0x8000 shift right 14 bits to have 2 = 0x0001. rate-1 = 14 -> rate=15.
    */
    int32_t rates[NUM_RATE] = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    int indicator = 0; // indicator for finding whether setfrequency is in valid frequency array
    unsigned int rate;
    unsigned int i;
    if (set_frequency < 0) { // RTC could not generate interrupts smaller than 0 Hz
        return -1;
    } else if (set_frequency > 1024) { // RTC could not generate interrupts larger than 1024 Hz
        return -1;
    } else {
        for(i = 0; i < NUM_RATE; i++) { // loop through valid frequency array
            if (set_frequency == valid_frequency[i]) { // check whether the setfrequency is a valid frequency
                indicator = 1;
                break;
            }
        }
        if (indicator == 0) { // not in valid frequency array
            return -1;
        } else {
            for(i = 0; i < NUM_RATE; i++) { 
                if (set_frequency == valid_frequency[i]) { // find location of value of setfrequency
                    rate = rates[i]; // get the rate from rates array
                    break;
                }
            }
        }
    }
    change_interrupt_rate(rate); // call helper function to change the interrupt rate
    return 0;
}

/* change_interrupt_rate
* Description: This function is used to change the rate.
* Input: rate -- new rate value
* Output: None
* Return value: None
* Side effect: None
*/
void change_interrupt_rate(unsigned int rate) {
    outb(REGISTER_A, RTC_PORT);	// Choose register A, and disable NMI
    unsigned char prev = inb(CMOS_PORT); // Read current value to register A      
    outb(REGISTER_A, RTC_PORT);	// reset index to A    
    outb((prev & MASK_TOP_FOUR) | rate, CMOS_PORT); // write only our rate to A. Note, rate is the bottom 4 bits.
    enable_irq(RTC_PIC_NUM); 
}


