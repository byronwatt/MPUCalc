OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)


MEMORY
{
    OCR_DEV       (RWX) : ORIGIN = 0x400000 LENGTH = 0x208000
    OCR_CTXT      (RWX) : ORIGIN = 0x608000 LENGTH = 0xB0000  /* 704K. must match address in device_ocr_addr.h */
    OCR_DESCRMEM  (RWX) : ORIGIN = 0x6B8000 LENGTH = 0x800    /* 1K Allocated to Packet Memory */
    OCR_PKTMEM    (RWX) : ORIGIN = 0x6c0000 LENGTH = 0x38000  /* ~204K Allocated to Packet Memory */
    /* note the mpu map for OCR_PKTMEM occupies two mpu entries
       // 006b8000 006bffff    32K  4 UNCACHED e.g. inbox/outbox, pktmem
       // 006c0000 006f7fff   224K  3 UNCACHED e.g. inbox/outbox, pktmem
       if we decrease the OCR_PKTMEM size to 224K, we can save an mpu entry
    */
    OCR_SHMEM     (RWX) : ORIGIN = 0x6F8000 LENGTH = 0x7C00   /* 16k for inbox/outbox plus some constants. must match address in device_ocr_addr.h AND Configure_MPU() in system_startup.cpp */
    OCR_MUTEXES   (RWX) : ORIGIN = 0x6FFC00  LENGTH = 0x300    /* must match address in device_ocr_addr.h AND Configure_MPU() in system_startup.cpp */
    OCR_FW_VER    (RWX) : ORIGIN = 0x6FFF00  LENGTH = 0x40     /* must match address in device_ocr_addr.h */
}

SECTIONS
{
    .vectors :
    {
       KEEP(*(.Vectors))
       . = ALIGN(4);
    } > OCR_DEV

    .text :
    {
       . = ALIGN(4);
       *(.text)     /* code */
       *(.text*)    /* code */
       *(.glue_7)   /* glue arm to thumb code */
       *(.glue_7t)  /* glue thumb to arm_code */
       . = ALIGN(4);
        _end_of_text = .; /* indicates end of text region */
    } > OCR_DEV
    
    .rodata :
    {
       . = ALIGN(4);
       *(.rodata)
       *(.rodata*)
    } > OCR_DEV
    
    .mpu_table :
    {
       . = ALIGN(4);
       *(.mpu_table)
       *(.mpu_table*)
        _end_of_rodata = .; /* indicates end of read only memory */
    } > OCR_DEV
    
    .ctors :
    {
        /* used by gcc to find constructors */
        /* .ctors */
        KEEP(*crtbegin.o(.ctors))
        KEEP(*crtbegin?.o(.ctors))
        KEEP(*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*crtend.o(.ctors))
        . = ALIGN(4);
    } > OCR_DEV

    .dtors :
    {
        
        /* used by gcc to find destructors */
        /* .dtors */
        KEEP(*crtbegin.o(.dtors))
        KEEP(*crtbegin?.o(.dtors))
        KEEP(*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
        KEEP(*(SORT(.dtors.*)))
        KEEP(*crtend.o(.dtors))
        . = ALIGN(4);
    } > OCR_DEV

    .init_fini :
    {
        /* sections needed for __libc_init_array to work */
        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);
    } > OCR_DEV

    .preinit_array :
    {
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
    } >OCR_DEV

    .init_array :
    {
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
    } >OCR_DEV

    .fini_array :
    {
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(4);

    } > OCR_DEV

    .ARM.extab (NOLOAD): 
    { 
      . = ALIGN(4);
      *(.ARM.extab* .gnu.linkonce.armextab.*)
      . = ALIGN(4);
    } > OCR_DEV

    .ARM.exidx (NOLOAD): 
    {
      . = ALIGN(4);
      __exidx_start = .;
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
      __exidx_end = .;
      . = ALIGN(4);
    } > OCR_DEV

    _etext = .; /* global symbol for end of code */

/*
since we want to protect .text,.rodata from accidental write access we will need to have .data start
on a new cache line (e.g. at least ALIGN(32))

if you need to configure some memory of size <256K then you need 4 pages to get up to 64 bytes resolution.

MPU regions are divided into 8 subregions, so if the start address is naturally aligned (starts with a bunch of zeros, then the MPU coverage for each region is:)

first page covers  256K +/-  32k
second page covers  32k +/-   4k
third page covers    4k +/- 512b
fourth page covers 512b +/-  64b

so if we only want to use 3 pages we should align to 512 bytes

probably not a big deal to align to 4k or even 32k and save another page or two depending on how we want to use mpu pages.
*/

    /* add named region to show how much memory is wasted,... otherwise the linker doesn't
     show how much memory is wasted because of the ALIGN() macro */
    .mpu_pad_ro (NOLOAD):
    {
        . = ALIGN(2048);
    } > OCR_DEV
    
    /* initialized data section */
    .data :
    {
        . = ALIGN(2048);
        __data_load = LOADADDR(.data);
        _sdata = .;
        __data_start__ = .; /* note: __data_start__ is used to calculate the read only section in */
        *(.data_begin .data_begin.*)

        *(.data)
        *(.data*)

        *(.data_end .data_end.*)
        . = ALIGN(4);

        _edata = .;  /* STM definition? */
        __data_end__ = .;
    } > OCR_DEV

    /* uninitialized data section */
    .bss (NOLOAD):
    {
        . = ALIGN(4);
        __bss_start__ = .;
        _sbss = .;
        *(.bss_begin .bss_begin.*)

        *(.bss)
        *(.bss*)
        *(COMMON)

        *(.bss_end .bss_end.*)
        . = ALIGN(4);
        __bss_end__ = .;
        _ebss = .;
    } > OCR_DEV

    .task_stacks (NOLOAD) :
    {
        /* this needs to match STACK_ALIGNMENT_IN_BYTES in multitask.h */
        . = ALIGN(256);
        *(.task_stacks)
        . = ALIGN(4);
    } > OCR_DEV

    .defects (NOLOAD) :
    {
        . = ALIGN(4);
        *(.defects)
        . = ALIGN(4);
    } > OCR_DEV

    .sw_defects (NOLOAD) :
    {
        . = ALIGN(4);
        *(.sw_defects)
        . = ALIGN(4);
    } > OCR_DEV

    .pmon_addr (NOLOAD) :
    {
        . = ALIGN(4);
        *(.pmon_address)
        . = ALIGN(4);
    } > OCR_DEV

    .pmon_values (NOLOAD) :
    {
        . = ALIGN(4);
        *(.pmon_values)
        . = ALIGN(4);
    } > OCR_DEV

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        __end__ = .;
        PROVIDE ( end = .);
        PROVIDE ( _end = .);
        *(.heap)
        . = ALIGN(8);
        __HeapLimit = .;
    } > OCR_DEV

    /* add named region to show how much memory is wasted,... otherwise the linker doesn't
     show how much memory is wasted because of the ALIGN() macro */
    .mpu_pad (NOLOAD):
    {
        . = ALIGN(32768);
    } > OCR_DEV

    .logging (NOLOAD) :
    {
/*
since we want to configure .logging as write-through we will benefit from having the start and end addresses "naturally aligned"

the start will need to be at least 32b aligned, but perhaps we can use 1024 byte aligned for now.

if you need to configure some memory of size <512K then you need 4 pages to get up to 64 bytes resolution.

MPU regions are divided into 8 subregions, so if the end address is naturally aligned (starts with a bunch of zeros, then the MPU coverage for each region is:)

first page covers   512K +/-   64k
second page covers   64k +/-    8k
third page covers     8k +/- 1024b
fourth page covers 1024b +/-  128b
*/
        . = ALIGN(32768);
        __logging_start__ = .;
        *(.logging)
        . = ALIGN(4);
    } > OCR_DEV

    .stats (NOLOAD) :
    {
        . = ALIGN(4);
        *(.stats)
        . = ALIGN(4);
    } > OCR_DEV

    .ctxt_space (NOLOAD) :
    {
        . = ALIGN(4);
        __logging_end__ = .; /* logging end is put here to make it a nice naturally aligned address */
        _start_of_ctxt = .; /* indicates start of ctxt */
        *(.ctxt)
        . = ALIGN(4);
    } > OCR_CTXT

    .shared_mem (NOLOAD) :
    {
        . = ALIGN(4);
        _start_of_shmem = .; /* indicates start of shmem */
        *(.ocr_shared_memory)
        . = ALIGN(4);
    } > OCR_SHMEM 

    .mutexes (NOLOAD) :
    {
        . = ALIGN(4);
        *(.ocr_mutexes)
    } > OCR_MUTEXES

    .fw_version (NOLOAD) :
    {
        . = ALIGN(4);
        *(.fw_fixed_version)
    } > OCR_FW_VER

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    *(.comment)
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }

  .stab       0 (NOLOAD) : { *(.stab) }
  .stabstr    0 (NOLOAD) : { *(.stabstr) }

}
