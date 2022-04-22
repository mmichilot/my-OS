#ifndef _PS2_H_
#define _PS2_H_

// PS/2 Ports
#define PS2_CMD     0x64
#define PS2_DATA    0x60
#define PS2_STATUS  PS2_CMD

// PS/2 Commands
#define PS2_DISABLE_PORT1   0xAD
#define PS2_DISABLE_PORT2   0xA7
#define PS2_READ_CONFIG     0x20
#define PS2_WRITE_CONFIG    0x60
#define PS2_CTRL_TEST       0xAA
#define PS2_PORT1_TEST      0xAB
#define PS2_PORT1_EN        0xAE

// PS/2 Controller Configuration
#define PS2_CFG_P1_INT      (1 << 0)
#define PS2_CFG_P2_INT      (1 << 1)
#define PS2_CFG_P1_CLK      (1 << 4)
#define PS2_CFG_P2_CLK      (1 << 5)
#define PS2_CFG_P1_TRANS    (1 << 6)

// PS/2 Status
#define PS2_STATUS_OUTPUT   (1 << 0)
#define PS2_STATUS_INPUT    (1 << 1)

// PS/2 Controller Test Responses
#define PS2_CTRL_TEST_SUCCESS   0x55
#define PS2_PORT1_TEST_SUCCESS  0x00

// PS/2 Keyboard Commands
#define PS2_KB_RST          0xFF
#define PS2_KB_SCANCODE_SET 0xF0
#define PS2_KB_SCAN_ENABLE  0xF4
#define PS2_KB_SCAN_DISABLE 0xF5

// PS/2 Keyboard Response
#define PS2_KB_RST_SUCCESS  0xAA
#define PS2_KB_ACK          0xFA
#define PS2_KB_RETRY        0xFE

// Scancode sets
#define GET_SCANCODE_SET    0
#define SCANCODE_SET_1      1
#define SCANCODE_SET_2      2
#define SCANCODE_SET_3      3

// Scancodes
#define KEY_RELEASED    0xF0
#define CAPS_LOCK       0x58
#define LEFT_SHIFT      0x12
#define RIGHT_SHIFT     0x59
#define LEFT_CONTROL    0x14
#define LEFT_ALT        0x11

#define PS2_FLUSH() while((inb(PS2_STATUS) & PS2_STATUS_OUTPUT)) { inb(PS2_DATA); }
extern void PS2_init(void);
extern void PS2_keyboard_poll(void);

#endif