#include <stdint-gcc.h>
#include "string.h"
#include "vga.h"

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
static struct vga_settings vga;
static int cursor;
static int blink;

// Local functions
void scroll();

/* --- VGA API --- */

void VGA_init(void) {
    // Initialize settings
    vga.fg_color = WHITE;
    vga.bg_color = BLACK;
    vga.bg_color = 0; // disable blinking

    // Set cursor to zero
    cursor = 0;

    // Clear screen
    VGA_clear();
}

void VGA_clear(void) 
{
    memset(vga_buf, 0, WIDTH*HEIGHT*sizeof(uint16_t));
    cursor = 0;
}

void VGA_char(unsigned char c) 
{   
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

void VGA_blink(void)
{
    int i;
    if(blink)
        vga_buf[cursor].bg = LIGHT_GRAY;
    else
        vga_buf[cursor].bg = vga.bg_color;

    for(i = 0; i < 100000000; i++);
    blink = !blink;
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