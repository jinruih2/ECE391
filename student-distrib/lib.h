/* lib.h - Defines for useful library functions
 * vim:ts=4 noexpandtab
 */

#ifndef _LIB_H
#define _LIB_H
#define BLANK_CHAR '\0'
#define BUF_SIZE    128
#define NUM_COLS    80
#define NUM_ROWS    25
#include "types.h"

// clear video memory
void clear(void);

// claer screen and update cursor
void clear_helper();

// print helper
int32_t printf(int8_t *format, ...);

// output a string on the screen
int32_t puts(int8_t *s);

// output a character on the screen
void putc(uint8_t c);

// scroll_up the screen
void scroll_up();

// clear the last row
void clearln();

// update the cursor location on the screen
void update_cursor();

// modified version of putc, supporting scrolling
void putc_modified(uint8_t c);

// modified version of putc_modified, supporting muti terminals
void putc_to_terminal(uint8_t c);

// delete a character
void deletc();

// delete a tab
void delete_tab();

// convert a number into ASCII
int8_t *itoa(uint32_t value, int8_t* buf, int32_t radix);

// reverse a string
int8_t *strrev(int8_t* s);

// get the length of the string
uint32_t strlen(const int8_t* s);

// set n consecutive bytes of s to value c
void* memset(void* s, int32_t c, uint32_t n);

// set lower 16 bits of n consecutive bytes of s to value c
void* memset_word(void* s, int32_t c, uint32_t n);

// set n consecutive memory locations of s to value c
void* memset_dword(void* s, int32_t c, uint32_t n);

// copy n bytes from source to destination
void* memcpy(void* dest, const void* src, uint32_t n);

// move n bytes from source to destination
void* memmove(void* dest, const void* src, uint32_t n);

// compare strings
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n);

// copy source string to destination
int8_t* strcpy(int8_t* dest, const int8_t*src);

// copy n consecutive bytes of source string to destination
int8_t* strncpy(int8_t* dest, const int8_t*src, uint32_t n);

// helper function to test rtc
void test_interrupts(void);

// update cursor for given x & y
void set_screen_xy(uint32_t x, uint32_t y);

// get screen_x
int get_screenx(void);

// get screen_y
int get_screeny(void);

/* Userspace address-check functions */
int32_t bad_userspace_addr(const void* addr, int32_t len);
int32_t safe_strncpy(int8_t* dest, const int8_t* src, int32_t n);

/* Port read functions */
/* Inb reads a byte and returns its value as a zero-extended 32-bit
 * unsigned int */
static inline uint32_t inb(port) {
    uint32_t val;
    asm volatile ("             \n\
            xorl %0, %0         \n\
            inb  (%w1), %b0     \n\
            "
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}

/* Reads two bytes from two consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them zero-extended
 * */
static inline uint32_t inw(port) {
    uint32_t val;
    asm volatile ("             \n\
            xorl %0, %0         \n\
            inw  (%w1), %w0     \n\
            "
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}

/* Reads four bytes from four consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them */
static inline uint32_t inl(port) {
    uint32_t val;
    asm volatile ("inl (%w1), %0"
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}

/* Writes a byte to a port */
#define outb(data, port)                \
do {                                    \
    asm volatile ("outb %b1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Writes two bytes to two consecutive ports */
#define outw(data, port)                \
do {                                    \
    asm volatile ("outw %w1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Writes four bytes to four consecutive ports */
#define outl(data, port)                \
do {                                    \
    asm volatile ("outl %l1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Clear interrupt flag - disables interrupts on this processor */
#define cli()                           \
do {                                    \
    asm volatile ("cli"                 \
            :                           \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Save flags and then clear interrupt flag
 * Saves the EFLAGS register into the variable "flags", and then
 * disables interrupts on this processor */
#define cli_and_save(flags)             \
do {                                    \
    asm volatile ("                   \n\
            pushfl                    \n\
            popl %0                   \n\
            cli                       \n\
            "                           \
            : "=r"(flags)               \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Set interrupt flag - enable interrupts on this processor */
#define sti()                           \
do {                                    \
    asm volatile ("sti"                 \
            :                           \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Restore flags
 * Puts the value in "flags" into the EFLAGS register.  Most often used
 * after a cli_and_save_flags(flags) */
#define restore_flags(flags)            \
do {                                    \
    asm volatile ("                   \n\
            pushl %0                  \n\
            popfl                     \n\
            "                           \
            :                           \
            : "r"(flags)                \
            : "memory", "cc"            \
    );                                  \
} while (0)

#endif /* _LIB_H */
