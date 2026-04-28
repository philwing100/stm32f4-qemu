MEMORY
{
  FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = 1024K
  RAM (rwx)       : ORIGIN = 0x20000000, LENGTH = 192K
}

SECTIONS
{
  .text :
  {
    KEEP(*(.isr_vector))
    *(.text*)
    *(.rodata*)
  } > FLASH

  .data : AT (ADDR(.text) + SIZEOF(.text))
  {
    *(.data*)
  } > RAM AT > FLASH

  .bss :
  {
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  } > RAM

  __text_start = ADDR(.text);
  __text_end = ADDR(.text) + SIZEOF(.text);
  __data_start = ADDR(.data);
  __data_end = ADDR(.data) + SIZEOF(.data);
  __data_load = LOADADDR(.data);
  __bss_start = ADDR(.bss);
}
