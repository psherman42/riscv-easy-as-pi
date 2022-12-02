# RISC-V As Easy As PI
### Presented at the Flash Memory Summit, 2022 Aug

## First Things, First

Low voltage supply (can) quickly kill an SD card, especially when it’s used in a development system (assembler, compiler, linker, loader)

Use ~5.25 V, 2.5A supply with good, thick 20 AWG cables, such as www.adafruit.com/product/1995

## Installing the O/S on a Raspberry PI

To Clean an older SD card, if needed:
> START --> Run --> diskpart --> List Disk --> Select disk x  
> List Partition --> Select partition x --> Delete partition  
> Create Partition Primary --> Format fs=fat32  

Get the *Raspberry PI Imager* program from raspberrypi.com/software
> Choose OS --> Raspberry PI OS (Other) --> Raspberry PI OS Lite (32-bit)  
> Choose STORAGE --> Generic STORAGE DEVICE USB DEVICE  
> (gear) set hostname, uid, pwd, wifi, locale as desired  
> WRITE  

Put the newly imaged SD card into the PI, plug in the PI, and follow commands below.

`sudo rasp-config – Localization [*] en_US UTF-8`

Edit two files using the `sudo vi` editor. Disable `bt` and `wifi` to save power, and only if you are using a direct connection to keyboard and ethernet.

```
/boot/cmdline.txt : console=tty1 root=... rootfstype=ext4 fsck.repair=yes
                    quiet loglevel=3 logo.nologo rootwait
```

```
/boot/config.txt : disable_splash=1
                   dtparam=audio=off
                   camera-auto-detect=0
                   enable_uart=1
                   dtoverlay=[pi3-]disable-bt
                   dtoverlay=[pi3-]disable-wifi
```

The following line prevents the start-up warning message *WiFi is currently blocked by rfkill*

`sudo sed –i ‘2i\ \ \ \ \ \ \ \ exit 0’ /etc/profile.d/wifi-check.sh`

```
sudo apt-get update
sudo apt-get install autoconf automake autotools-dev curl python3 git
             libmpc-dev libmpfr-dev libgmp-dev
             gawk build-essential bison flex texinfo gperf
             libtool patchutils bc zlib1g-dev libexpat-dev
             libfdt-dev libisl-dev
sudo apt clean
sudo apt autoremove
```

For best Linux filesystem and SD flash memory card health: **DON’T!** pull the plug before you `sudo shutdown now`

## Building the “Tool Chain” for RISC-V

***Assembler, Compiler, Linker***

**DO NOT** use the *many thread* `-j` option, it is too hard on the SD flash memory card.
```
sudo rm –fr /opt/riscv32
sudo rm –fr ./riscv-gnu-toolchain
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
mkdir x-rv32imac-ilp32
cd x-rv32imac-ilp32
../configure –prefix=/opt/riscv32 --enable-languages=c,c++
                                  --with-arch=rv32imac
                                  --with-abi=ilp32
sudo make
export RISCV=/opt/riscv32
export PATH=$PATH:$RISCV/bin
```

***Loader***

```
sudo apt-get install libusb-1.0-0 libusb-1.0-0-dev
sudo rm –fr ./openocd
git clone git://git.code.sf.net/p/openocd/code openocd
cd openocd
./bootstrap
./configure –prefix=/opt/openocd --enable-bcm2835gpio --enable-sysfsgpio
make
sudo make install
```

If all goes well, you can test your shiny new toolchain versions like so:

```
riscv32-unknown-elf-gcc --version   <== should show something like 11.1.0
riscv32-unknown-as --version                                       2.38
riscv32-unknown-ld --version                                       2.38
riscv32-unknown-gdb --version                                      10.1
openocd --version                                                  0.11.0
```

## Configuring the Hardware

## Wiring the Hardware

| GPIO  6: | 31 ------- 5 | :TCK  |
| GPIO 13: | 33 ------- 7 | :TMS  |
| GPIO 26: | 37 ------- 8 | :TDI  |
| GPIO  5: | 29 ------- 4 | :TCK  |
| GPIO 12: | 32 ------- 6 | :TCK  |
|     GND: | 39 ------- 28 | :TCK  |
| UART TX: | 8  ------- 20 | :UART0.RX (GPIO 17)  |
| UART RX: | 10 ------- 21 | :UART0.TX (GPIO 16)  |
| GPIO 17: | 11 ------- 15 | :SPI1.SS2 (GPIO 9) |
| GPIO 27: | 13 ------- 16 | :SPI1.SS3 (GPIO 10) |
| GPIO 22: | 15 ------- 17 | :PWM2.1 (GPIO X)  |


## Assembling, Compiling, Linking, and Loading

***Assembling, Compiling, Linking -- the Makefile***

The MAKEFILE script does not need to be changed when switching between Flash and RAM 
boot or code execution.

Notice the two places where the linker script file “foo.lds” gets used in the build process.

***Linker Script***

This is how to selectively load and/or boot from Flash (**ROM**) or **RAM**. It is a bit bare but should be easy to see all of the moving parts.

There are only two places to change when making the choice between Flash (**ROM**) or **RAM**: The linker script file “foo.lds” shown here, and the Loading & Running command lines, shown next.

```
OUTPUT_ARCH(“riscv”)
ENTRY( _start_ )
MEMORY`
{
    rom : ORIGIN = 0x20000000, LENGTH = 0x2000
    ram (rxa!ri) : ORIGIN = 0x80000000, LENGTH = 0x4000
}
SECTIONS
{
    .text : { *(.text*) } > ram ... or ... rom
    .rodata : { *(.rodata*) } > ram ... or ... rom
    .bss : { *(.bss*) } > ram
}
```

***Loader Script***

Interface specification – How to tell OpenOCD which pins and wires of the *host system* to use.

`jtag_nums # # # #` is where you define the connection signals: `TCK TMS TDI TDO` in that order! Note that these are gpio port numbers, *not* physical connector pin numbers.

```
rpi-3b.cfg adapter driver bcm2835gpio
           bcm2835gpio peripheral_base 0x3f000000
           bcm2835gpio speed_coeffs 97469 24
           bcm2835gpio jtag_nums 6 13 26 5
           bcm2835gpio swd_nums 6 13
           bcm2835gpio srst_nums 12
           reset_config srst_only separate srst_nogate
```

Target specification – How to tell OpenOCD what kind of chip to talk to.

```
fe310-g002.cfg transport select jtag
               jtag newtap riscv cpu –irlen 5 –expected-id 0x20000913
               target create riscv.cpu.0 riscv –chain-position riscv.cpu
               riscv.cpu.0 configure –work-area-phys 0x80000000
                                     -work-area-size 0x100000
                                     -work-area-backup 0
```

***Loading & Running***

The Load command line needs to change in two places when switching between **RAM** or Flash (**ROM**) boot, as shown by the use of the `load_image` and `verify_image` statements, and the `flash bank` and `flash write_image` commands.

The Run command line needs to change in one places when switching between **RAM** or Flash (**ROM**) boot, as shown by the target address `0x80000000` and `0x20000000`.

An encapsulation of all of the necessary steps, including great improvement in speed and efficiency of flashing code into ROM, is available at https://github.com/psherman42/demystifying-openocd

__***Load***__ **RAM**

```
sudo openocd –f rpi-3b.cfg –f fe310-g002.cfg –c “adapter speed 1000”
             –c init
             –c “reset init”
             –c “sleep 25”
             –c “riscv set_reset_timeout_sec 25”
             –c “adapter speed 2500”
             –c “load_image foo.bin 0x80000000 bin”
             –c “verify_image foo.bin 0x80000000 bin”
             –c shutdown –c exit
```

__***Load***__ **ROM**

```
sudo openocd –f rpi-3b.cfg –f fe310-g002.cfg
             –c “flash bank spi0 fespi 0x20000000 0 0 0 riscv.cpu.0 0x10014000”
             –c “adapter speed 1000”
             –c init
             –c “reset init”
             –c “sleep 25”
             –c “riscv set_reset_timeout_sec 25”
             –c “adapter speed 2500”
             –c “flash write_image erase unlock foo.bin 0x20000000 bin”
             –c shutdown –c exit
```

__***Run***__ **RAM**

```
sudo openocd –f rpi-3b.cfg –f fe310-g002.cfg –c “adapter speed 1000”
             –c init
             –c “reset init”
             –c “sleep 25”
             –c “adapter speed 2500”
             –c “resume 0x80000000” 
             –c shutdown –c exit
```

__***Run***__ **ROM**

```
sudo openocd –f rpi-3b.cfg –f fe310-g002.cfg –c “adapter speed 1000”
             –c init
             –c “reset init”
             –c “sleep 25”
             –c “adapter speed 2500”
             –c “resume 0x20000000”
             –c shutdown –c exit
```

## What Can Go Wrong

**Load & Run Successful**

```
Info : Examined RISC-V core: found 1 harts
Info :  hart 0: XLEN=32, misa=0x40101105
```

**Load & Run Unsuccessful**

`Error: Fatal: Hart 0 failed to halt during examine()`

or

```
Error executing event examine-start on target riscv.cpu.0
Error: DMI operation didn't complete in 2 seconds. The target is either really slow or broken. You could increase the timeout with riscv set_command_timeout_sec.
```

Both indicate the possibility of JTAG not reset, possibly due to insufficient reset pulse timing, low voltage, or noise supply lines such as from bad ground connections.

## Sample Program

Demonstration for *Simple Terminal* and *Linux Logic Analyzer* following below. Send characters `F`, `M`, `S`, `f`, `m`, and `s` in any order and watch the output in the Terminal and the Analyzer.

```
#include <stdint.h> // for uint32_t
#include <stddef.h> // for size_t
#include “clock.h"  // for clock_init()
#include “uart0.h”  // for uart0_...()
#include “gpio.h”  // for gpio_...()

unsigned int x;
uint32_t clk_hz;

void main() {
    clk_hz = clock_init( PRCI_EXT_DIR );
    gpio_init();
    uart0_init( clk_hz, 115200 );
    uart0_write_string( “welcome to uart test\r\n”);
    gpio_dir( 9, GPIO_OUT ); gpio_dir( 10, GPIO_OUT ); gpio_dir( 11, GPIO_OUT );
    while(1) {
        x = uart_read();
        switch( x ) {
            case ‘F’: uart0_write_string(“Flash “); gpio_high( 9 ); break;
            case ‘f’: uart0_write_string(“flash “); gpio_low( 9 ); break;
            case ‘M’: uart0_write_string(“Memory “); gpio_high( 10 ); break;
            case ‘m’: uart0_write_string(“memory “); gpio_low( 10 ); break;
            case ‘S’: uart0_write_string(“Summit “); gpio_high( 11 ); break;
            case ‘s’: uart0_write_string(“summit “); gpio_low( 11 ); break;
            case ‘\r’: uart0_write_string(“\r\n”); break;
            default: uart0_write( (uint8_t *) &x, 1); break;
        }
    }
}
```

## Simple Terminal

`sudo ~/prj/boot/term.sh /dev/serial0 115200`

![image/simple-term](https://user-images.githubusercontent.com/36460742/184531061-d63deebf-061f-41b8-8b69-95e41ea14af5.jpg)

Available at https://github.com/psherman42/simple-term

## Linux Logic Analyzer

```
sudo ~/prj/boot/sense.sh --c1 17 --c2 27 --c3 22
                         --tc 17 --tp + --tm norm
                         --cl1 GPIO17 --cl2 GPIO-27 --cl3 GPIO-22
```

![Linux Logic Analyzer](https://user-images.githubusercontent.com/36460742/184530503-dff819aa-8683-4606-90f7-7425a1cf5a06.jpg)

Where
> `c1`, `c2`, `c2` – channel GPIO pin(s)  
> `tc` – trigger channel GPIO pin  
> `tp` – trigger polarity (+ or -)  
> `tm` – trigger mode (auto or norm)  
> `cl1`, `cl2`, `cl3` – channel label(s)  

Available at https://github.com/psherman42/linux-logic-analyzer

##Further Reading

**SiFive Docs** – `https://www.sifive.com/documentation`
> __E31 Core Complex Manual__, Freedom E310 __Datasheet__ & __Manual__  
> https://forums.sifive.com (good technical discussion, see HiFive1 Rev B, user: **pds**)  
> https://github.com/sifive/sifive-blocks  (complete rtl and scala design)  

**LoFive R1** – `https://github.com/mwelling/lofive`

**RPi** – https://pinout.xyz https://www.raspberrypi.com/software

**USB Adapters**: Olimex, FTDI FT-2232, etc.

**Availability**: digikey, mouser, adafruit

## Is RISC Five as easy as Mac or PC?

**It sure is!** Use the FT(2)232 chip with any USB port.
> Mac - drivers already supported  
> PC - may need to disable the UEFI driver security check  

JTAG Reset line glitches at startup, so revise a little bit as shown below.

The setting `layout_init 0x0808 0x0a1b` shows the bug, which is a tiny 10uS glitch on nTRST at startup. Instead, the setting `layout_init 0x0b08 0x0b1b` fixes the bug, by specifying the rst lines as *outputs* with *push-pull* drive.

```
ftdi.cfg adapter driver ftdi
         ftdi device_desc "Olimex OpenOCD JTAG ARM-USB-TINY-H“
         ftdi vid_pid 0x15ba 0x002a
         
          #----------------- P/U –-- DIR --
          #ftdi layout_init 0x0808 0x0a1b
           ftdi layout_init 0x0b08 0x0b1b
           
           ftdi layout_signal nSRST -oe 0x0200
           ftdi layout_signal nTRST -data 0x0100 -oe 0x0100
           ftdi layout_signal LED -data 0x0800
```

The `layout_init` setting words are defined by MPSSE below:

```
Sig  MPSSE  PIN    BIT P/U DIR
---  -----  ---    --- --- ---
TCK  TCK/SK ADBUS0  0   0   1
TDI  TDI/DO ADBUS1  1   0   1
TDO  TDO/DI ADBUS2  2   0   0
TMS  TMS/CS ADBUS3  3   1   1
???  GPIOL0 ADBUS4  4   0   1
.    GPIOL1 ADBUS5  5   0   0
.    GPIOL2 ADBUS6  6   0   0
.    GPIOL3 ADBUS7  7   0   0
TRST GPIOH0 ACBUS0  8   1   1  <== P/U and DIR fix the bug noted above
SRST GPIOH1 ACBUS1  9   1   1  <== P/U fixes the bug noted above
.    GPIOH2 ACBUS2  a   0   0
LED  GPIOH3 ACBUS3  b   1   1
.    GPIOH4 ACBUS4  c   0   0
.    GPIOH5 ACBUS5  d   0   0
.    GPIOH6 ACBUS6  e   0   0
.    GPIOH7 ACBUS7  f   0   0
```

