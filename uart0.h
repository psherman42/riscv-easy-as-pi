#ifndef UART_H__INCLUDED
#define UART_H__INCLUDED

#include <stdint.h>
#include <stddef.h>

void uart0_init (uint32_t clk_hz, uint32_t baud);
void uart0_write (uint8_t *buf, size_t len);
void uart0_write_string (char *str);
uint8_t uart0_read (void);

#endif//UART_H__INCLUDED
