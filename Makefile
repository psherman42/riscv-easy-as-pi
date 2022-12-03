

RISCVGNU ?= riscv32-unknown-elf

AOPS = -march=rv32imac -mabi=ilp32
COPS = -march=rv32imac -mabi=ilp32 -Wall -O2 -nostdlib -nostartfiles -ffreestanding

all : uart.bin

clean :
	rm -f *.o
	rm -f *.elf
	rm -f *.bin
	rm -f *.lst
	rm -f *.hex
	rm -f *.map

start.o : start.s
	$(RISCVGNU)-as $(AOPS) start.s -o start.o

clock.o : clock.c
	$(RISCVGNU)-gcc $(COPS) -c clock.c -o clock.o

util.o : util.c
	$(RISCVGNU)-gcc $(COPS) -c util.c -o util.o

gpio.o : gpio.c
	$(RISCVGNU)-gcc $(COPS) -c gpio.c -o gpio.o

uart0.o : uart0.c
	$(RISCVGNU)-gcc $(COPS) -c uart0.c -o uart0.o

main.o : main.c
	$(RISCVGNU)-gcc $(COPS) -c main.c -o main.o

uart.bin : uart.lds start.o util.o clock.o gpio.o uart0.o main.o
	$(RISCVGNU)-ld start.o util.o clock.o gpio.o uart0.o main.o -T uart.lds -o uart.elf -Map uart.map
	$(RISCVGNU)-objdump -D uart.elf > uart.lst
	$(RISCVGNU)-objcopy uart.elf -O ihex uart.hex
	$(RISCVGNU)-objcopy uart.elf -O binary uart.bin
