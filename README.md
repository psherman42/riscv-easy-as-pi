# RISC Five As Easy As PI

RISC-V is the new thing on the block. Here we show how to build up the lemonade stand, using only those everyday things you can find around home. We go through the basic process of compiling, assembling, linking, and loading; describe the basics how JTAG works (and how it fails); and do it all from the comfort of one's Pi [or computer] that happens to be lying around collecting dust. Reduced Instruction Set Computing is a simple thing, deserving of the reduced development platform. The basic knowledge learned from this presentation will serve both the data storage professional and the computer enthusiast  well for many years - and many variants - to come.

The novice and budding hardware enthusiast, who wishes to get their hands dirty and their feet wet, yet has only a few pennies in their pocket with which to spend, will enjoy this beginner-level presentation. Fundamental basics are covered in a fun and simple way using vernacular and practices of modern technology. By the end of this talk, everyone will be able to blink a light and display [store and retrieve] "Hello, World".

This presentation will show and remind all of us just what are the basic blocks and steps necessary for any development endeavor, in a simple and easy to follow manner. All knowledge and tips are completely self-contained, without reliance on any fancy tool or third party product. Thus, the reader gains fundamental knowledge which will be transferrable in timeless fashion for many years to come.

<sub><sup>Paul Sherman is a computer engineer in the Silicon Valley with concerns for the problems of the people of the world. He has played an active role in the evolution of the data storage industry for the past 25 years, working with companies such as Western Digital and Seagate. He has published many articles with the SAS Institute user group community on statistical problems, co-authored numerous articles and books on progressive economics, and received four U.S. patents for inventions on testing and manufacture of data storage devices. He is an avid evangelist for MRAM. He earned an MS in Physics from University of California Irvine and a BA from University of California Santa Cruz.</sup><sub>

<sub><sup>Presented at the *Flash Memory Summit, 2022 Aug* https://flashmemorysummit.com</sup><sub>

## First Things, First

Low voltage supply can and/or will quickly kill an SD card, especially when it’s used in a development system (assembler, compiler, linker, loader)

Use ~**5.25 V**, 2.5A supply with good, thick 20 AWG cables, such as www.adafruit.com/product/1995

Prevents this

<img src="https://user-images.githubusercontent.com/36460742/205295723-e8557d09-5874-4f97-ae45-d052d7470b2f.png" width="50">

and this

<img src="https://user-images.githubusercontent.com/36460742/205295642-12ecacb4-4d19-42ff-8eac-16d11a8aae36.png" width="300">


## Installing the O/S on a Raspberry PI

To Clean an older SD card, if needed, using the Windows system:
> START &rarr; Run &rarr; diskpart &rarr; List Disk &rarr; Select disk x  
> &rarr; List Partition &rarr; Select partition x &rarr; Delete partition  
> &rarr; Create Partition Primary &rarr; Format fs=fat32  

Get the *Raspberry PI Imager* program from www.raspberrypi.com/software
> Choose OS &rarr; Raspberry PI OS (Other) &rarr; Raspberry PI OS Lite (32-bit)  
> &rarr; Choose STORAGE &rarr; Generic STORAGE DEVICE USB DEVICE  
> &rarr; (gear) set hostname, uid, pwd, wifi, locale as desired  
> &rarr; WRITE  

Put the newly imaged SD card into the PI, plug in the PI, and follow commands below.

`sudo rasp-config` &rarr; Localization [\*] en_US UTF-8

Edit two files using the `sudo vi` editor. Disable `bt` and `wifi` to save power, and only do so if you are using a direct connection to keyboard and ethernet.

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

The following line prevents the start-up warning message *WiFi is currently blocked by rfkill*. Use this command if you decide to disable `bt` and `wifi`.

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

For best Linux filesystem and SD flash memory card health: **DON’T** pull the plug before you `sudo shutdown now`

## Building the “Tool Chain” for RISC-V

***Assembler, Compiler, Linker***

**DO NOT** use the *many thread* `-j` option of `make`, it is too hard on the SD flash memory card.
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

The toolchain builds in the following sequence: `binutils` &rarr; `gcc` &rarr; `newlib` &rarr; `gdb`

If all goes well, you can test your shiny new toolchain versions like so:

```
riscv32-unknown-elf-gcc --version   <== should show something like 11.1.0
riscv32-unknown-as --version                                       2.38
riscv32-unknown-ld --version                                       2.38
riscv32-unknown-gdb --version                                      10.1
openocd --version                                                  0.11.0
```

## Wiring the Hardware

It takes only a few wires to connect a PI to a RISC-V chip. Throw in a few more if you wish to use serial or parallel I/O pins like the UART, SPI, I2C, or GPIO ports.
                                            
Oh, and please don't forget one wire for signal ground.

|  RPi    |               | LoFive-R1          |
|    ---: |     :---:     | :---               |
| GPIO  6 | 31 -------  5 | TCK                <td rowspan="6">JTAG connections</td> |
| GPIO 13 | 33 -------  7 | TMS                |
| GPIO 26 | 37 -------  8 | TDI                |
| GPIO  5 | 29 -------  4 | TDO                |
| GPIO 12 | 32 -------  6 | SRST               |
|     GND |  39 ===== 28  | GND                |
| UART TX | 8  ------- 20 | UART0.RX (GPIO 17) <td rowspan="2">UART serial port</td> |
| UART RX | 10 ------- 21 | UART0.TX (GPIO 16) |
| GPIO 17 | 11 ------- 15 | SPI1.SS2 (GPIO 9)  <td rowspan="3">GPIO parallel port</td> |
| GPIO 27 | 13 ------- 16 | SPI1.SS3 (GPIO 10) |
| GPIO 22 | 15 ------- 17 | PWM2.1 (GPIO X)    |

***Physical pinout***

```
    RPi (3B+)                     LoFive-R1
+---------------+              +----------------+
|    Display    |              |  1          28 | <== note square pads on both pins 1 and 28
|               |              |  2          27 |
|          1  2 |              |  3          26 |
| USB      3  4 |              |  4          25 |
|          5  6 |              |  5          24 |
|          7  8 |              |  6          23 |
|          9 10 |              |  7          22 |
|         11 12 |              |  8          21 |
| HDMI    13 14 |              |  9          20 |
|         15 16 |              | 10          19 |
|         17 18 |              | 11          18 |
|         29 20 |              | 12          17 |
|         21 22 |              | 13          16 |
|         23 24 |              | 14          15 |
|         25 26 |              +----------------+
|         27 28 |
|         39 30 |
|         31 32 |
|         33 34 |
|         35 36 |
|         37 38 |
|         39 40 |
|               |
| LAN     USB   |
+---------------+
```

## Assembling, Compiling, Linking, and Loading

***Assembling, Compiling, Linking -- the Makefile***

The MAKEFILE script `foo.mk` does not need to be changed when switching between Flash and RAM 
boot or code execution.

Notice the two places where the linker script file `foo.lds` gets used in the build process.

`foo.mk`:
```
RISCVGNU ?= riscv32-unknown-elf
AOPS = -march=riscv32imac –mabi=ilp32
COPS = -march=riscv32imac –mabi=ilp32 –Wall –O2 –nostdlib –nostartfiles –ffreestanding

start.o : start.s
       $(RISCVGNU)-as $(AOPS) start.s –o start.o

... all other ASM and C source files go here ...

main.o : main.c
       $(RISCVGNU)-gcc $(COPS) –c main.c –o main.o

foo.bin : foo.lds start.o ... main.o
       $(RISCVGNU)-ld start.o ... main.o –T foo.lds –o foo.elf –Map foo.map
       $(RISCVGNU)-objdump –D foo.elf > foo.lst
       $(RISCVGNU)-objcopy foo.elf –O ihex foo.hex
       $(RISCVGNU)-objcopy foo.elf –O binary foo.bin

clean:
       rm –f *.o
       rm –f *.elf
       rm –f *.bin
       rm –f *.lst
       rm –f *.hex
       rm –f *.map
```

Note that indented lines are with a __single tab character__, not many spaces, as standard practice for any MAKEFILE.

***Linker Script***

This is how to selectively load and/or boot from Flash (**ROM**) or **RAM**. It is a bit bare but should be easy to see all of the moving parts.

There are only two places to change when making the choice between Flash (**ROM**) or **RAM**: The linker script file “foo.lds” shown here, and the Loading & Running command lines, shown next.

`foo.lds`
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

It is easiest to make a pair of linker script files, suffixed with `-ram` and `rom`. That way, you don't need to keep re-editing the linker script file and risk accidentally breaking something.

***Loader Script***

THere are two main parts here, the physical wiring connections and the logical target device definition. They are mutually exclusive, and you can keep each in its own configuration file as shown.

Interface specification – How to tell OpenOCD which pins and wires of the *host system* to use.

`jtag_nums # # # #` is where you define the four connection signals: `TCK TMS TDI TDO` in that order! Note that these are gpio port numbers, *not* physical connector pin numbers. Similarly for `swd_nums # #` which defines the two connection signals `SWCLK SWDIO` in that order.

`rpi-3b.cfg`:
```
adapter driver bcm2835gpio
bcm2835gpio peripheral_base 0x3f000000
bcm2835gpio speed_coeffs 97469 24
bcm2835gpio jtag_nums 6 13 26 5
bcm2835gpio swd_nums 6 13
bcm2835gpio srst_nums 12
reset_config srst_only separate srst_nogate
```

Target specification – How to tell OpenOCD what kind of chip to talk to.

`fe310-g002.cfg`:
```
transport select jtag
jtag newtap riscv cpu –irlen 5 –expected-id 0x20000913
target create riscv.cpu.0 riscv –chain-position riscv.cpu
riscv.cpu.0 configure –work-area-phys 0x80000000
                      -work-area-size 0x100000
                      -work-area-backup 0
```

***Loading & Running***

The Load command line needs to change in two places when switching between **RAM** or Flash (**ROM**) boot, as shown by the use of the `load_image` and `verify_image` statements, and the `flash bank` and `flash write_image` commands.

The Run command line needs to change in one places when switching between **RAM** or Flash (**ROM**) boot, as shown by the target address `0x80000000` and `0x20000000`.

An encapsulation of all of the necessary steps, including physical wiring connections, logical target device definition, target device memory loading, and target device running, as well as great improvement in speed and efficiency of flashing code into ROM, is available at https://github.com/psherman42/demystifying-openocd. See the section below, *Can I do it all with one click (or key press)?* for further explanation.

__***Load to***__ **RAM**

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

__***Load to***__ **ROM**

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

__***Run from***__ **RAM**

```
sudo openocd –f rpi-3b.cfg –f fe310-g002.cfg –c “adapter speed 1000”
             –c init
             –c “reset init”
             –c “sleep 25”
             –c “adapter speed 2500”
             –c “resume 0x80000000” 
             –c shutdown –c exit
```

__***Run from***__ **ROM**

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

A message like this is usually accompanied by a beer or other celebration.

```
Info : Examined RISC-V core: found 1 harts
Info :  hart 0: XLEN=32, misa=0x40101105
```

**Load & Run Unsuccessful**

Anguish and melancholy arise when you see these. Don't despair. Both indicate the possibility of JTAG not reset, possibly due to insufficient reset pulse timing, low voltage, or noise supply lines such as from bad ground connections, and are easily remedied.

In rare cases a hard power reset of the target might be needed; see discussion of [Understanding the PRCI Clock Path](https://forums.sifive.com/t/understanding-the-prci-clock-path/5827/2) in the SiFive forums.

`Error: Fatal: Hart 0 failed to halt during examine()`

or

```
Error executing event examine-start on target riscv.cpu.0
Error: DMI operation didn't complete in 2 seconds. The target is either really slow or broken. You could increase the timeout with riscv set_command_timeout_sec.
```

## Sample Program

Demonstration for *Simple Terminal* and *Linux Logic Analyzer* following below. Send characters `F`, `M`, `S`, `f`, `m`, and `s` in any order and watch the output in the Terminal and the Analyzer. All of the source files for an FE310 SoC are included in this repository.

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

Probably one of the world's smallest terminal emulators. Run in a separate session (Alt-F2, etc) for best results.

`sudo ~/prj/boot/term.sh /dev/serial0 115200`

<img src="https://user-images.githubusercontent.com/36460742/184531061-d63deebf-061f-41b8-8b69-95e41ea14af5.jpg" width="700" alt="Simple Terminal">

Available at https://github.com/psherman42/simple-term

## Linux Logic Analyzer

It's neither fast nor fancy but it shows what happens when. Run in a separate session (Alt-F3, etc) for best results.

```
sudo ~/prj/boot/sense.sh --c1 17 --c2 27 --c3 22
                         --tc 17 --tp + --tm norm
                         --cl1 GPIO17 --cl2 GPIO-27 --cl3 GPIO-22
```

Where
> `c1`, `c2`, `c2` – channel GPIO pin(s)  
> `tc` – trigger channel GPIO pin  
> `tp` – trigger polarity (+ or -)  
> `tm` – trigger mode (auto or norm)  
> `cl1`, `cl2`, `cl3` – channel label(s)  

<img src="https://user-images.githubusercontent.com/36460742/184530503-dff819aa-8683-4606-90f7-7425a1cf5a06.jpg" width="700" alt="Linux Logic Analyzer">

Available at https://github.com/psherman42/linux-logic-analyzer

## Further Reading

**SiFive Docs** – https://www.sifive.com/documentation
> E31 __Core Complex Manual__  
> E310 __Datasheet__  
> E310 __Manual__  

**SiFive Technical Discussion** - https://forums.sifive.com (see HiFive1 Rev B, user: **pds**)  

**SiFive Hardware Design** - https://github.com/sifive/sifive-blocks  (complete set of rtl and scala files)  

**LoFive R1** – https://github.com/mwelling/lofive

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

## Can I do it all with one click (or key press)?

**Yes!**

`make –f foo.mk ram –tgt=LOAD`

The link step is invoked by the `-ld` command, and the load step is invoked by the (optional) `openocd` command, shown in the *ram* target below. Assembling and Compiling steps are not shown, for clarity.

`my-proj.mk`:
```
ram : foo.lds start.o ... main.o
       $(RISCVGNU)-ld start.o ... main.o -T foo.lds -o foo.elf -Map foo.map
       $(RISCVGNU)-objdump -D foo.elf > foo.lst
       $(RISCVGNU)-objcopy foo.elf -O ihex foo.hex
       $(RISCVGNU)-objcopy foo.elf -O binary foo.bin
ifeq ($(tgt), LOAD)
       @openocd -f interface/ftdi/olimex-arm-usb-tiny-h.cfg -f foo.cfg
                                                            -c init -c "asic_ram_load foo“
                                                            -c shutdown -c exit
else
       @echo "target not changed“
endif
```

Indented lines are with a __single tab character__, not many spaces.

Note the `@` symbol to run a shell command from within a makefile.

See [Demystifying OpenOCD](https://github.com/psherman42/Demystifying-OpenOCD) for more information and a full working example.

## Any Other Questions or Comments?

Post them to the *Issues* of this repo!
