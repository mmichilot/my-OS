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
static struct vga_entry *vga_buff = (struct vga_entry*) VGA_BASE;
static struct vga_settings vga;
static int cursor;

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
    memset(vga_buff, 0, WIDTH*HEIGHT*sizeof(uint16_t));
    cursor = 0;
}

void VGA_char(unsigned char c) 
{
    if (c == '\n') {
        cursor = (LINE(cursor) + 1) * WIDTH;
    }
    else {
        vga_buff[cursor].ascii = c;
        vga_buff[cursor].fg = vga.fg_color;
        vga_buff[cursor].bg = vga.bg_color;
        cursor++;
    }

    if (cursor >= WIDTH*HEIGHT)
        scroll();
}

int VGA_str(const char *str) 
{
    int i;

    for (i = 0; str[i] != '\0'; i++)
        VGA_char(str[i]);

    return i;
};

void VGA_fg_color(enum VGA_COLOR fg) {
    vga.fg_color = fg;
}

void VGA_bg_color(enum VGA_COLOR bg) {
    vga.bg_color = bg;
}

/* --- Internal Functions --- */
void scroll()
{
    // Move lines 2 to HEIGHT up one
    memcpy(vga_buff, vga_buff+WIDTH, WIDTH*(HEIGHT-1)*sizeof(uint16_t));

    // Clear last line
    memset(vga_buff+WIDTH*(HEIGHT-1), 0, WIDTH*sizeof(uint16_t));

    // Set cursor back to the start of the last line
    cursor = WIDTH*(HEIGHT-1);
}