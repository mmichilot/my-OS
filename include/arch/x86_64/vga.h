#ifndef _VGA_H_
#define _VGA_H_

// VGA color codes
enum VGA_COLOR {
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GRAY,
    DARK_GRAY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    PINK,
    YELLOW,
    WHITE
};

extern void VGA_init(void);
extern void VGA_clear(void);
extern void VGA_char(char c);
extern void VGA_str(const char *str);
void VGA_fg_color(enum VGA_COLOR fg);
void VGA_bg_color(enum VGA_COLOR bg);

#endif