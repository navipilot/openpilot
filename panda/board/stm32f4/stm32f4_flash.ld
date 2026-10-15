/* SPDX-License-Identifier: MIT */
ENTRY(Reset_Handler)

/* Preserve the bootstub/application handoff word used by existing DOS firmware. */
enter_bootloader_mode = 0x2001FFFC;
_estack = 0x2001FFFC;
_app_start = 0x08004000;

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1536K
  RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K - 4
  CANRAM (xrw) : ORIGIN = 0x20020000, LENGTH = 192K
}

SECTIONS
{
  .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
  .text :
  {
    *(.text*) *(.rodata*) *(.glue_7) *(.glue_7t)
    KEEP(*(.init)) KEEP(*(.fini))
    . = ALIGN(4);
  } > FLASH
  .ARM.extab : { *(.ARM.extab*) } > FLASH
  .ARM.exidx : { *(.ARM.exidx*) } > FLASH
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT > FLASH
  _sidata = LOADADDR(.data);
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss*) *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM
  /* Ring indices live in initialized RAM; queue storage needs no initial value. */
  .canram (NOLOAD) : { *(.canram*) } > CANRAM
  ASSERT(_ebss + 0x1000 <= _estack, "Insufficient DOS stack space")
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
