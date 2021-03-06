#include <stdarg.h>
#include <stdint-gcc.h>
#include "string.h"
#include "drivers/vga.h"
#include "drivers/serial.h"

static inline void num_to_string(char *str, unsigned long long num, int sign, int base);

void print_int(char *str, int num);
void print_uint(char *str, unsigned int num, int base);
void print_short(char *str, short num);
void print_ushort(char *str, unsigned short num, int base);
void print_long(char *str, long num);
void print_ulong(char *str, unsigned long num, int base);
void print_long_long(char *str, long long num);
void print_ulong_long(char *str, unsigned long long num, int base);

int print_str(const char *str) {
    if (is_serial_enabled())
        SER_write(str, strlen(str));
    
    if (is_vga_enabled())
        VGA_str(str);

    return strlen(str);
}

int print_char(char c) {
    if (is_serial_enabled())
        SER_write(&c, 1);
    
    if (is_vga_enabled())
        VGA_char(c);

    return 1;
}

int printk(const char *fmt, ...)
{
    va_list va;
    int num_chars = 0;
    char c = '\0';
    char buf[256];

    va_start(va, fmt);

    // Loop through each character in format string
    while ((c = *fmt++) != '\0') {
        // Parse any labels
        if (c == '\001') {
            c = *fmt++;
            num_chars += print_char('[');
            switch(c) {
                case '1': // Error
                    VGA_fg_color(RED);
                    num_chars += print_str("ERROR");
                    break;
                case '2': // Info
                    num_chars += print_str("INFO");
                    break;
                default:
                    break;
            }
            VGA_fg_color(WHITE);
            num_chars += print_str("] ");
        }
        // Parse specifiers
        else if (c == '%') {
            c = *fmt++;
            switch (c) {
                case '%':
                    print_char('%');
                    num_chars++;
                    break;
                case 'c':
                    print_char((unsigned char) va_arg(va, int));
                    num_chars++;
                    break;
                case 's':
                    num_chars += print_str(va_arg(va, const char*));
                    break;
                case 'p':
                    num_chars += print_str("0x");
                    print_ulong(buf, va_arg(va, unsigned long), 16);
                    num_chars += print_str(buf);
                    break;
                
                // Integer
                case 'd':
                    print_int(buf, va_arg(va, int));
                    num_chars += print_str(buf);
                    break;
                case 'u':
                    print_uint(buf, va_arg(va, unsigned int), 10);
                    num_chars += print_str(buf);
                    break;
                case 'x':
                    print_uint(buf, va_arg(va, unsigned int), 16);
                    num_chars += print_str(buf);
                    break;

                // Short
                case 'h':
                    c = *fmt++;
                    switch (c) {
                        case 'd':
                            print_short(buf, (short) va_arg(va, int));
                            num_chars += print_str(buf);
                            break;
                        case 'u':
                            print_ushort(buf, (unsigned short) va_arg(va, unsigned int), 10);
                            num_chars += print_str(buf);
                            break;
                        case 'x':
                            print_ushort(buf, (unsigned short) va_arg(va, unsigned int), 16);
                            num_chars += print_str(buf);
                            break;
                        default:
                            print_char(c);
                            num_chars++;
                            break;
                    }
                    break;

                // Long
                case 'l':
                    c = *fmt++;
                    switch (c) {
                        case 'd':
                            print_long(buf, va_arg(va, long));
                            num_chars += print_str(buf);
                            break;
                        case 'u':
                            print_ulong(buf, va_arg(va, unsigned long), 10);
                            num_chars += print_str(buf);
                            break;
                        case 'x':
                            print_ulong(buf, va_arg(va, unsigned long), 16);
                            num_chars += print_str(buf);
                            break;
                        default:
                            num_chars += print_char(c);
                            break;
                    }
                    break;

                    // Long long
                    case 'q':
                    c = *fmt++;
                    switch (c) {
                        case 'd':
                            print_long_long(buf, va_arg(va, long long));
                            num_chars += print_str(buf);
                            break;
                        case 'u':
                            print_ulong_long(buf, va_arg(va, unsigned long long), 10);
                            num_chars += print_str(buf);
                            break;
                        case 'x':
                            print_ulong_long(buf, va_arg(va, unsigned long long), 16);
                            num_chars += print_str(buf);
                            break;
                        default:
                            num_chars += print_char(c);
                            break;
                    }
                    break;


                default:
                    num_chars += print_char(c);
                    break;
            }
        }
        // Print character
        else {
            num_chars += print_char(c);
        }
    }

    va_end(va);
    return num_chars; 
}

/* Converts an number into a string */
static inline void num_to_string(char *str, unsigned long long num, int neg, int base)
{
    int r = 0,i = 0;
    char buf[256];

    // Convert value to a string
    if (num == 0)
        buf[i++] = '0';
    else {
        while (num != 0) {
            r = num % base;
            if (base == 16 && r >= 10)
                buf[i++] = (r - 10) + 'a';
            else 
                buf[i++] = r + '0';
            num = num / base;
        }
    }

    // Insert negative sign
    if (neg)
        buf[i++] = '-';

    // Reverse string
    while (i-- > 0)
        *str++ = buf[i];

    // Append null character
    *str = '\0';
}

void print_int(char *str, int num)
{
    int neg = 0;

    if (num < 0) {
        num = -num;
        neg = 1;
    }

    num_to_string(str, num, neg, 10);
}

void print_uint(char *str, unsigned int num, int base)
{
    num_to_string(str, num, 0, base);
}

void print_short(char *str, short num)
{
    int neg = 0;
    
    if (num < 0) {
        num = -num;
        neg = 1;
    }

    num_to_string(str, num, neg, 10);
}

void print_ushort(char *str, unsigned short num, int base)
{
    num_to_string(str, num, 0, base);
}

void print_long(char *str, long num)
{
    int neg = 0;
    
    if (num < 0) {
        num = -num;
        neg = 1;
    }

    num_to_string(str, num, neg, 10);
}

void print_ulong(char *str, unsigned long num, int base)
{
    num_to_string(str, num, 0, base);
}

void print_long_long(char *str, long long num)
{
    int neg = 0;

    if (num < 0) {
        num = -num;
        neg = 1;
    }

    num_to_string(str, num, neg, 10);
}

void print_ulong_long(char *str, unsigned long long num, int base)
{
    num_to_string(str, num, 0, base);
}