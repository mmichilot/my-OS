#include "string.h"
#define BUFF_SIZE 1024

struct State {
    char buff[BUFF_SIZE];
    char *consumer, *producer;
};

void SER_init(void)
{
    static struct State ser_state;
    memset(ser_state.buff, 0, BUFF_SIZE);
    ser_state.consumer = &ser_state.buff[0];
    ser_state.producer = &ser_state.buff[0];
}