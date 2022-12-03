////////////////////////////////////////////////
//
// x
// 2020-04-02 pds    initial cut
//

#include <stdint.h>  // for uint32_t
#include <stddef.h>  // for size_t
#include "clock.h"
#include "uart0.h"
#include "gpio.h"


///////////////////////////////////////////////
//
// entry point
//

int main()
{
    unsigned int x;
    uint32_t clk_hz;

    clk_hz = clock_init( PRCI_EXT_DIR );  // first things, first

    gpio_init();

    uart0_init( clk_hz, 115200 );

    uart0_write_string( "welcome to uart test\r\n" );

    gpio_dir( 9, GPIO_OUT );
    gpio_dir( 10, GPIO_OUT );
    gpio_dir( 11, GPIO_OUT );

    while(1)
    {
      x = uart0_read();
      switch (x)
      {
        case 'F':
          uart0_write_string( "Flash " );
          gpio_high( 9 );
          break;
        case 'f':
          uart0_write_string( "flash " );
          gpio_low( 9 );
          break;
        case 'M':
          uart0_write_string( "Memory " );
          gpio_high( 10 );
          break;
        case 'm':
          uart0_write_string( "memory " );
          gpio_low( 10 );
          break;
        case 'S':
          uart0_write_string( "Summit " );
          gpio_high( 11 );
          break;
        case 's':
          uart0_write_string( "summit " );
          gpio_low( 11 );
          break;
        case '?':
          uart0_write_string( "welcome to uart test\r\n" );
          break;
        case '\r':
          uart0_write_string( "\r\n" );
          break;
        default:
          uart0_write( (uint8_t *) &x, 1 );
          break;
      }
    }

    return 0xDEADBEEF;
}
