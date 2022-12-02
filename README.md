# RISC-V As Easy As PI
### Presented at the Flash Memory Summit, 2022 Aug

## First Things, First

Low voltage supply (can) quickly kill an SD card, especially when it’s used in a development system (assembler, compiler, linker, loader)

Use ~5.25 V, 2.5A supply with good, thick 20 AWG cables, such as www.adafruit.com/product/1995

## Installing the O/S

## Building the “Tool Chain”

## Configuring the Hardware

## Wiring the Hardware

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
    **rom** : ORIGIN = 0x20000000, LENGTH = 0x2000
    **ram** (rxa!ri) : ORIGIN = 0x80000000, LENGTH = 0x4000
}
SECTIONS
{
    .text : { *(.text*) } > **ram** ... or ... **rom**
    .rodata : { *(.rodata*) } > **ram** ... or ... **rom**
    .bss : { *(.bss*) } > **ram**
}
```

***Loader Script***

Interface specification – How to tell OpenOCD which pins and wires of the *host system* to use.

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

The Load & Run command lines need to change in two places when switching between **RAM** or Flash (**ROM**) boot, as shown by the highlighted statements.

**[LOAD] RAM**

**[LOAD] ROM**

**[RUN] RAM**

**[RUN] ROM**

## What Can Go Wrong

**Load & Run Successful**

```
...
Info : Examined RISC-V core: found 1 harts
Info :  hart 0: XLEN=32, misa=0x40101105
...
```

**Load & Run Unsuccessful**

`Error: Fatal: Hart 0 failed to halt during examine()`

or

`Error executing event examine-start on target riscv.cpu.0`

`Error: DMI operation didn't complete in 2 seconds. The target is either really slow or broken. You could increase the timeout with riscv set_command_timeout_sec.`

Both indicate the possibility of JTAG not reset, possibly due to insufficient reset pulse timing, low voltage, or noise supply lines such as from bad ground connections.

## Simple Terminal

`sudo ~/prj/boot/term.sh /dev/serial0 115200`

Available at https://github.com/psherman42/simple-term

## Linux Logic Analyzer

```
sudo ~/prj/boot/sense.sh --c1 17 --c2 27 --c3 22
                         --tc 17 --tp + --tm norm
                         --cl1 GPIO17 --cl2 GPIO-27 --cl3 GPIO-22
```

Where

`c1`, `c2`, `c2` – channel GPIO pin(s)

`tc` – trigger channel GPIO pin

`tp` – trigger polarity (+ or -)

`tm` – trigger mode (auto or norm)

`cl1`, `cl2`, `cl3` – channel label(s)

Available at https://github.com/psherman42/linux-logic-analyzer

##Further Reading##

SiFive Docs – `https://www.sifive.com/documentation`

E31 Core Complex Manual, Freedom E310 Datasheet & Manual

```
https://forums.sifive.com/u/pds
https://github.com/sifive/sifive-blocks
```

LoFive R1 – `https://github.com/mwelling/lofive`

RPi – https://pinout.xyz
`https://www.raspberrypi.com/software`

USB Adapters: Olimex, FTDI FT-2232, etc.

Availability: digikey, mouser, adafruit
