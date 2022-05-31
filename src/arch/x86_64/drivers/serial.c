#include <stddef.h>
#include "printk.h"
#include "port-io.h"
#include "irq.h"

#define BUFF_SIZE 1024
#define COM1_PORT 0x3F8

// UART Registers
#define BASE COM1_PORT
// DLAB = 0
#define THR BASE
#define IER BASE+1
#define IIR BASE+2
#define LCR BASE+3
#define LSR BASE+5
//DLAB = 1
#define DLL BASE
#define DLM BASE+1

struct State {
    char buff[BUFF_SIZE];
    char *consumer, *producer;
};

static struct State *state = NULL;
static bool hw_idle = true;
static bool serial_enabled = false;

bool is_serial_enabled()
{
    return serial_enabled;
}

void start_tx(struct State *state) 
{
    // Check if buffer is empty
    if (state->consumer == state->producer) {
        return;
    }
    
    hw_idle = false;
    char c = *state->consumer++;
    outb(THR, c);

    // Wrap consumer around if needed
    if (state->consumer >= &state->buff[BUFF_SIZE])
        state->consumer = &state->buff[0];
}

int add_to_buff(char c, struct State *state) 
{
    if (state->producer == state->consumer - 1 ||
        (state->consumer == &state->buff[0] && state->producer == &state->buff[BUFF_SIZE-1]))
        return 0;

    *state->producer++ = c;
    
    // Wrap producer if needed
    if (state->producer >= &state->buff[BUFF_SIZE])
        state->producer = &state->buff[0];

    return 1;
}

/* Serial Write */
int SER_write(const char* buff, int len) 
{
    bool enable_ints = false;
    if (are_interrupts_enabled()) {
        enable_ints = true;
        cli();
    }

    // Poll to check if soft-state is valid
    if (!hw_idle) {
        if (inb(LSR) & 0x20)
            hw_idle = true;
    }
    
    // Add to buffer
    int i;
    int num_added = 0;
    for (i = 0; i < len; i++) {
        // Keep adding to buffer until everything is added
        // or buffer is full
        if(add_to_buff(buff[i], state) == 0)
            break;
        num_added++;
    }

    // Initiate transfer if hardware is idle
    if (!(state->consumer == state->producer) && hw_idle)
        start_tx(state);

    if (enable_ints)
        sti();

    return num_added;
}

/* Serial Interrupt Handler */
void SER_int(int error, void* arg) 
{
    UNUSED(error);
    UNUSED(arg);

    uint8_t cause = inb(IIR) & 0x0F;
    
    // LINE interrupt
    if (cause == 0x6)
        inb(LSR); // Read LSR to clear LINE interrupt
    // TX interrupt
    else if (cause == 0x2) {
        hw_idle = true;
        start_tx(state);
    }
    else
        pr_err("Unhandled Serial Interrupt\n");
}

struct State* init_state(void) 
{
    static struct State state;

    state.consumer = &state.buff[0];
    state.producer = &state.buff[0];

    return &state;
}

void init_hardware(void) 
{
    pr_info("Inititalizing COM1 Serial Device\n");

    outb(IER, 0x00); // Disable interrupts
    
    outb(LCR, 0x80); // Enable DLAB
    outb(DLL, 0x01); // Set speed to 115200 bps
    outb(DLM, 0x00);

    outb(LCR, 0x03); // Set 8N1 and disable DLAB

    outb(IER, 0x06); // Enable TX interrupts
}

void add_ISR(void)
{
    if (are_interrupts_enabled()) {
        cli();

        IRQ_set_handler(36, SER_int, NULL);
        pr_info("Registered Serial Interrupt Handler @ IRQ 36\n");
        sti();
    }
}

void SER_init()
{
    pr_info("Initializing Serial Device\n");
    state = init_state();
    init_hardware();
    add_ISR();
    serial_enabled = true;
    pr_info("Serial Device Initialized\n");
}

