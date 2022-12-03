#ifndef UART_H__INCLUDED
#define UART_H__INCLUDED

#include <stdint.h>
#include <stddef.h>

void tostr (char *str, size_t len, uint32_t val);
size_t strlen (char *str);

#endif//UART_H__INCLUDED
