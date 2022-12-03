OUTPUT_ARCH("riscv")

ENTRY( _start )

MEMORY
{
  rom : ORIGIN = 0x20000000, LENGTH = 0x2000
  ram (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 0x4000
}

SECTIONS
{
  .text :
  {
    PROVIDE(_text_start = .);
    *(.text*)
    PROVIDE(_text_end = .);
  }
  > ram

  . = ALIGN(4);

  .rodata :
  {
    PROVIDE(_rodata_start = .);
    *(.rodata*)
    PROVIDE(_rodata_end = .);
  }
  > ram

  . = ALIGN(4);

  .bss :
  {
    PROVIDE(_bss_start = .);
    *(.bss*)
    PROVIDE(_bss_end = .);
  }
  > ram
}
