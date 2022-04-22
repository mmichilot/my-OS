#include <stdint-gcc.h>
#include <stdbool.h>
#include "ps2.h"
#include "port-io.h"
#include "printk.h"
#include "scancode.h"

static void PS2_keyboard_init(void);
static inline void PS2_keyboard_cmd(uint8_t cmd);
static inline void PS2_cmd(uint8_t cmd);
static inline void PS2_write(uint8_t val);
static inline uint8_t PS2_read();

static const char shifted_key[] = {
    ['`'] = '~', ['1'] = '!', ['2'] = '@', 
    ['3'] = '#', ['4'] = '$', ['5'] = '%', 
    ['6'] = '^', ['7'] = '&', ['8'] = '*', 
    ['9'] = '(', ['0'] = ')', ['-'] = '_', 
    ['='] = '+', ['['] = '{', [']'] = '}',
    ['\\']= '|', [';'] = ':', ['\'']= '"', 
    [','] = '<', ['.'] = '>', ['/'] = '?'
};

/* Internal variables */
static bool caps_lock = false;
static bool shift = false;
static bool control = false;
static bool alt = false;

/*
 *  PS/2 Functions
 */
static inline void PS2_cmd(uint8_t cmd) {
    while(inb(PS2_STATUS) & PS2_STATUS_INPUT);
    outb(PS2_CMD, cmd);
}

static inline void PS2_write(uint8_t val) {
    while(inb(PS2_STATUS) & PS2_STATUS_INPUT);
    outb(PS2_DATA, val);
}

static inline uint8_t PS2_read() {
    while(!(inb(PS2_STATUS) & PS2_STATUS_OUTPUT));
    return (uint8_t) inb(PS2_DATA);
}

void PS2_init(void)
{
    pr_info("Initializing PS/2 Interface\n");

    PS2_cmd(PS2_DISABLE_PORT1);
    PS2_cmd(PS2_DISABLE_PORT2);

    PS2_cmd(PS2_READ_CONFIG);
    uint8_t config = PS2_read();

    PS2_FLUSH();

    // Modify controller configuration
    config &= ~(PS2_CFG_P1_CLK | PS2_CFG_P2_INT | PS2_CFG_P1_TRANS);
    PS2_cmd(PS2_WRITE_CONFIG);
    PS2_write(config);


    PS2_cmd(PS2_CTRL_TEST);
    if (PS2_read() != PS2_CTRL_TEST_SUCCESS) {
        pr_err("PS/2 Controller Self Test failed!\n");
        return;
    }

    // Reload controller configuration just in case
    PS2_cmd(PS2_WRITE_CONFIG);
    PS2_write(config);

    PS2_cmd(PS2_PORT1_TEST);
    if (PS2_read() != PS2_PORT1_TEST_SUCCESS) {
        pr_err("PS/2 Port 1 Interface Test failed!\n");
        return;
    }

    PS2_cmd(PS2_PORT1_EN);

    pr_info("PS/2 Interface Initialized\n");

    PS2_keyboard_init();

}

/*
 *  PS/2 Keyboard Functions
 */

static void PS2_keyboard_init(void)
{

    pr_info("Initializing PS/2 Keyboard\n");

    PS2_keyboard_cmd(PS2_KB_RST);
    if (PS2_read() != PS2_KB_RST_SUCCESS) {
        pr_err("Keyboard reset failed!\n");
        return;
    }

    PS2_keyboard_cmd(PS2_KB_SCANCODE_SET);
    PS2_keyboard_cmd(SCANCODE_SET_2);
    
    PS2_keyboard_cmd(PS2_KB_SCAN_ENABLE);

    pr_info("PS/2 Keyboard Initialized\n");
}

static inline void PS2_keyboard_cmd(uint8_t cmd) {
    uint8_t resp, retry = 0;

    // Attempt to write to keyboard 10 times
    do {
        PS2_write(cmd);
        resp = PS2_read();
    } while(resp == PS2_KB_RETRY && retry++ < 10);
    
    if (resp != PS2_KB_ACK)
        pr_err("PS2 keyboard write failed after 10 retries!\n");   
}

void PS2_keyboard_poll(void)
{
    uint8_t scancode;
    
    scancode = PS2_read();

    switch (scancode) {
        case KEY_RELEASED:
            scancode = PS2_read();
            if (scancode ==  LEFT_SHIFT || scancode == RIGHT_SHIFT) shift = false;
            if (scancode == LEFT_CONTROL) control = false;
            if (scancode == LEFT_ALT) alt = false;
            return;
        
        case CAPS_LOCK:
            caps_lock = !caps_lock;
            return;
        
        case LEFT_SHIFT:
        case RIGHT_SHIFT:
            shift = true;
            return;
        
        case LEFT_CONTROL:
            printk("Left Control\n");
            control = true;
            return;

        case LEFT_ALT:
            printk("Left Alt\n");
            alt = true;
            return;
    }

    char key = scancode_to_ascii(scancode);
    if (key >= 'a' && key <= 'z')
        key -= (caps_lock ^ shift) ? 32 : 0;
    else if (shift)
        key = shifted_key[(int) key];

    printk("%c", key);
}