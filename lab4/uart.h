#pragma once
#include "fila.h"

struct boot_resources;
#define WORD_MSB(x)			((x) >> 8)
#define WORD_LSB(x)			((x) & 0xFF)

#define RACINIX_SERIAL_PORT_NUMBER								1
#define RACINIX_SERIAL_TRIGGER_LEVEL							2
#define RACINIX_SERIAL_NUM_BITS									8
#define RACINIX_SERIAL_NUM_STOP_BITS							2
#define RACINIX_SERIAL_PARITY									0
#define RACINIX_SERIAL_BAUD_RATE								115200
#define UART_MAX_BITRATE						115200

int uart_subscribe_int(uint8_t *bit_no);
void self_clearing_receiver();
void self_clearing_transmitter();
int uart_unsubscribe_int();
int uart_ih();
int receive_queue();
int transmit_queue();

int serial_set( unsigned long bits, unsigned long stop, long parity, unsigned long rate);

fila *get_transmitter(); 
fila* get_receiver();


