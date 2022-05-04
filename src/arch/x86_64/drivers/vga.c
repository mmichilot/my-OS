#include <stdint-gcc.h>
#include "string.h"
#include "drivers/vga.h"
#include "irq.h"

#define VGA_BASE 0xb8000
#define WIDTH 80
#define HEIGHT 25
#define LINE(x) (x / WIDTH)

// VGA text buffer entry
struct vga_entry 
{
    uint8_t ascii;
    uint8_t fg:4;
    uint8_t bg:4;
} __attribute__((packed));

// VGA display settings
struct vga_settings
{
    enum VGA_COLOR fg_color;
    enum VGA_COLOR bg_color;
};

// Local VGA variables
static struct vga_entry *vga_buf = (struct vga_entry*) VGA_BASE;
static struct vga_settings vga = {.bg_color = BLACK, .fg_color = WHITE};
static int cursor = 0;

// Local functions
void scroll();

/* --- VGA API --- */

void VGA_init(void) {
    memset(vga_buf, 0, WIDTH*HEIGHT*sizeof(uint16_t));
}

void VGA_clear(void) 
{
    memset(vga_buf, 0, WIDTH*HEIGHT*sizeof(uint16_t));
    cursor = 0;
}

void VGA_char(unsigned char c) 
{   
    bool enable_ints = false;
    if (are_interrupts_enabled()) {
        enable_ints = true;
        cli();
    }

    // Do nothing for null character
    if (c == '\0')
        return;
    
    switch(c) {
        case 0x08: // Backspace
            // Move cursor to the last ASCII character in the prev. line
            if ((cursor % WIDTH) == 0)
                while(vga_buf[--cursor].ascii == '\0');
            else
                cursor--;

            vga_buf[cursor].ascii = '\0';
            break;
        case '\t':
            cursor = cursor + 4;
            break;
        case '\n':
            cursor = (LINE(cursor) + 1) * WIDTH;
            break;
        default:
            vga_buf[cursor].ascii = c;
            vga_buf[cursor].fg = vga.fg_color;
            vga_buf[cursor].bg = vga.bg_color;
            cursor++;
            break;
    }

    if (cursor >= WIDTH*HEIGHT)
        scroll();
    else if (cursor < 0)
        cursor = 0;

    if (enable_ints)
        sti();
}

int VGA_str(const char *str) 
{
    int i;

    for (i = 0; str[i] != '\0'; i++)
        VGA_char(str[i]);

    return i;
};

void VGA_fg_color(enum VGA_COLOR fg)
{
    vga.fg_color = fg;
}

void VGA_bg_color(enum VGA_COLOR bg)
{
    vga.bg_color = bg;
}

/* --- Internal Functions --- */
void scroll()
{
    // Move lines 2 to HEIGHT up one
    memcpy(vga_buf, vga_buf+WIDTH, WIDTH*(HEIGHT-1)*sizeof(uint16_t));

    // Clear last line
    memset(vga_buf+WIDTH*(HEIGHT-1), 0, WIDTH*sizeof(uint16_t));

    // Set cursor back to the start of the last line
    cursor = WIDTH*(HEIGHT-1);
}