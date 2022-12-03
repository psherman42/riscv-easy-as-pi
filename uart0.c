////////////////////////////////////////////////
// serial uart #1
//
// 2020-04-02 pds    initial cut
//

#include <stdint.h>  // for uint32_t
#include <stddef.h>  // for size_t
#include "defs.h"
#include "gpio.h"  // for iof_en,iof_sel


///////////////////////////////////////////////
//
// low level uart routines
//

#define uart0_base 0x10013000

//
// TXDATA
//

#define uart0_txdata   (*(volatile uint32_t *) (uart0_base + 0x00))

#define uart0_tx_data(x)  ( uart0_txdata = (( uart0_txdata & ~(0xFF << 0)) | (( (x) & 0xFF) << 0)) )

// 0=empty, 1=full
#define uart0_tx_full()  ( ((uart0_txdata >> 31) & 0x1) )

//
// RXDATA
//

#define uart0_rxdata   (*(volatile uint32_t *) (uart0_base + 0x04))

#define uart0_rx_data()  ( ((uart0_rxdata >> 0) & 0xFF) )

// 0=empty, 1=full
#define uart0_rx_empty()  ( ((uart0_rxdata >> 31) & 0x1) )

//
// TXCTRL
//

#define uart0_txctrl   (*(volatile uint32_t *) (uart0_base + 0x08))

// 0=dis, 1=ena
#define uart0_txen(x)  ( uart0_txctrl = (( uart0_txctrl & ~(0x1 << 0)) | (( (x) & 0x1) << 0)) )

// 0=one-stop-bit, 1=two-stop-bits
#define uart0_nstop(x)  ( uart0_txctrl = (( uart0_txctrl & ~(0x1 << 1)) | (( (x) & 0x1) << 1)) )

#define uart0_txcnt(x)  ( uart0_txctrl = (( uart0_txctrl & ~(0x7 << 16)) | (( (x) & 0x7) << 16)) )

//
// RXCTRL
//

#define uart0_rxctrl   (*(volatile uint32_t *) (uart0_base + 0x0C))

// 0=dis, 1=ena
#define uart0_rxen(x)  ( uart0_rxctrl = (( uart0_rxctrl & ~(0x1 << 0)) | (( (x) & 0x1) << 0)) )

#define uart0_rxcnt(x)  ( uart0_rxctrl = (( uart0_rxctrl & ~(0x7 << 16)) | (( (x) & 0x7) << 16)) )

//
// IE
//

#define uart0_ie       (*(volatile uint32_t *) (uart0_base + 0x10))

// 0=dis, 1=ena
#define uart0_ie_txwm(x)  ( uart0_ie = (( uart0_ie & ~(0x1 << 0)) | (( (x) & 0x1) << 0)) )

// 0=dis, 1=ena
#define uart0_ie_rxwm(x)  ( uart0_ie = (( uart0_ie & ~(0x1 << 1)) | (( (x) & 0x1) << 1)) )

//
// IP
//

#define uart0_ip       (*(volatile uint32_t *) (uart0_base + 0x14))

// 0=none, 1=pending
#define uart0_ip_txwm()  ( (( uart0_ip >> 0) & 0x1) )

// 0=none, 1=pending
#define uart0_ip_rxwm()  ( (( uart0_ip >> 1) & 0x1) )

//
// DIV
//

#define uart0_sckdiv   (*(volatile uint32_t *) (uart0_base + 0x18))

// n = (hfclk / baud) - 1
#define uart0_div(x)  ( uart0_sckdiv = (( uart0_sckdiv & ~(0xFFFF << 0)) | (( (x) & 0xFFFF) << 0)) )


///////////////////////////////////////////////
//
// high level uart routines
//

void uart0_init (uint32_t clk_hz, uint32_t baud)
{
  gpio_pin_sel( 16 , 0 );  // iof sel uart0, rx, 48-QFN pin 38, input to SoC
  gpio_pin_sel( 17 , 0 );  // iof sel uart0, tx, 48-QFN pin 39, output from SoC

  gpio_pin_en( 16 , 1 );  // iof ena
  gpio_pin_en( 17 , 1 );  // iof ena

  /////////////////
  // after copy/paste, for uart1()
  //gpio_pin_sel( 23 , 0 );  // iof sel uart1, rx, 48-QFN pin 45, input to SoC
  //gpio_pin_sel( 18 , 0 );  // iof sel uart1, tx, 48-QFN pin 40, output from SoC

  //gpio_pin_en( 23 , 1 );  // iof ena
  //gpio_pin_en( 18 , 1 );  // iof ena
  // after copy/paste, for uart1()
  /////////////////

  //uart0_div( 119 );  // (13800000 / 115200) - 1, hfclk=13.8MHz
  //uart0_div( 138 );  // (16000000 / 115200) - 1, hfclk=16.0MHz
  uart0_div( (clk_hz / baud) -1 );  // (16000000 / 115200) - 1, hfclk=16.0MHz
  uart0_txen( 1 );
  uart0_nstop( 0 );  // one stop bit
  uart0_rxen( 1 );
  uart0_rxcnt( 0 );  // one byte
}

void __uart0_write (uint8_t data)
{
  while( uart0_tx_full() );
  uart0_tx_data( data );
}

void uart0_write (uint8_t *buf, size_t len)
{
  int i;
  for (i=0; i<len; i++)
  {
    __uart0_write( buf[i] );
  }
}

void uart0_write_string (char *str)
{
  size_t len;

  len = 0;
  while (str[len] != '\0') len++;  // strlen(), thread safe

  uart0_write( (uint8_t *) str, len );
}

uint8_t uart0_read (void)
{
  uint8_t data;

  while( uart0_ip_rxwm() == 0 );  // wait for ch (blocking read)
  data = uart0_rx_data();

  return data;
}

