# MPUCalc
ARM Cortex M7 Memory Protection Unit Calculator and Display

:pick: This project is under construction !

## resources

[ARM CMSIS](https://www.keil.com/pack/doc/CMSIS/Core/html/group__mpu__functions.html#details)

[ARM Cortex-M7 Devices Generic User Guide r1p1 MPU access permission attributes ](https://developer.arm.com/documentation/dui0646/b/BIHIIJDC)

[Chris Coleman fix-bugs-and-secure-firmware-with-the-mpu](https://interrupt.memfault.com/blog/fix-bugs-and-secure-firmware-with-the-mpu)

[Chris Coleman arm-cortex-m-exceptions-and-nvic](https://interrupt.memfault.com/blog/arm-cortex-m-exceptions-and-nvic)

[Jean Labrosse using-a-memory-protection-unit-with-an-rtos-part-2](https://www.embeddedcomputing.com/technology/processing/using-a-memory-protection-unit-with-an-rtos-part-2)

[usefulness-of-mpu-in-a-non-os-system](https://community.arm.com/support-forums/f/architectures-and-processors-forum/7107/usefulness-of-mpu-in-a-non-os-system)

[Niall Cooling setting-up-the-cortex-m34-armv7-m-memory-protection-unit-mpu](https://blog.feabhas.com/2013/02/setting-up-the-cortex-m34-armv7-m-memory-protection-unit-mpu/)

[STMicroelectronics managing-memory-protection-unit-in-stm32-mcus-stmicroelectronics](https://www.st.com/resource/en/application_note/dm00272912-managing-memory-protection-unit-in-stm32-mcus-stmicroelectronics.pdf)

## introduction

The ARM Cortex-M7 Memory Protection Unit (MPU) is fantastic for embedded applications, and the CMSIS functions to program the entries are well designed and very easy to use.

But unfortunately the D-Stream product does not display the resulting memory ranges, and programming a few entries to cover a range of memory is also left as an exercise for the student.

## our enhancements

At microchip, we added some functions to display the memory protection settings for all of memory, and a calculator to program entries to cover a range of memory.

I found that looking up the values of TEX/B/C and figuring out what that combination of values really meant was confusing, so I created macros for the TEX/B/C values that I use, and use these descriptions of the settings rather than arbitrary TEX/B/C settings in all the tools.  This means that it looks nice to someone that doesn't want to know the gory details, but it's a layer of potential confusion.  (e.g. i use the name "NO_ACCESS" instead of TEX=0,B=0,C=0)

Calculating the memory protection settings for a particular range of memory is a little expensive, (a bit of code & temporary heap), so rather than calculate the settings when the program starts, instead we have a second program that looks at the linker sections and creates the MPU entries, then we recompile one file with the updated MPU entries and relink. 

## FreeRTOS stack red zone.

we also updated FreeRTOS task create/switch to program a red zone at the bottom of the stack so that we get a memory fault immediately if stack overflows.

## example:

For our case we would program the MPU entries to be read/only & execute for the .text section, and read/write & no-execute for the .bss section, plus we had some write-through memory for logging, some uncached memory for inbox/outbox, and some device memory for the IO space.

After programming the entries, displaying the memory map looks like this:

```
start    end      size   #  description   
-------- -------- ------ -- -----------   
00000000 003fffff     4M  . unmapped      
00400000 0044ffff   320K  5 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)           
00450000 004517ff     6K  6 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)           
00451800 0048d7ff   240K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)     
0048d800 0048ffff    10K  8 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)             
00490000 004effff   384K  7 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)             
004f0000 004f7fff    32K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)     
004f8000 004fbfff    16K  4 UNCACHED e.g. inbox/outbox                    
004fc000 004fffff    16K  3 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)             
00500000 00efffff    10M  . unmapped      
00f00000 00ffffff     1M  1 DEVICE_SHAREABLE                  
01000000 02ffffff    32M  0 DEVICE_SHAREABLE                  
03000000 fffffffc     4G  . unmapped 
```

The calculated MPU entries looks like this:
```c
    // DEV_CFG
    // 0: 0x00000000, size=64M, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x0, B=0x1, SRD=0xc3
    //    subregion_size=8M, subregions=0x3c
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    N    0x00000000 0x007fffff
    //       1   0x02    N    0x00800000 0x00ffffff
    //       2   0x04    Y    0x01000000 0x017fffff <-- enabled
    //       3   0x08    Y    0x01800000 0x01ffffff <-- enabled
    //       4   0x10    Y    0x02000000 0x027fffff <-- enabled
    //       5   0x20    Y    0x02800000 0x02ffffff <-- enabled
    //       6   0x40    N    0x03000000 0x037fffff
    //       7   0x80    N    0x03800000 0x03ffffff
    {
        .RBAR = ARM_MPU_RBAR(0UL, 0x00000000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, DEVICE_SHAREABLE, 0xc3, ARM_MPU_REGION_SIZE_64MB)
    },
    // DEV_CFG
    // 1: 0x00f00000, size=1M, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x0, B=0x1, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(1UL, 0x00f00000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, DEVICE_SHAREABLE, 0x0, ARM_MPU_REGION_SIZE_1MB)
    },
    // OCR (default is shareable and works for LDREX/STREX, does it work for jtag?)
    // 2: 0x00400000, size=1M, XN=1, AP=0x3, TEX=0x1, S=0x1, C=0x1, B=0x1, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(2UL, 0x00400000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, 0x0, ARM_MPU_REGION_SIZE_1MB)
    },
    // executable and read only for both .text and .rodata (__data_start__ = 0044f800)
    // 3: 0x00400000, size=256K, XN=0, AP=0x6, TEX=0x1, S=0x1, C=0x1, B=0x1, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(3UL, 0x00400000UL),
        .RASR = ARM_MPU_RASR_EX(EXECUTE, ARM_MPU_AP_RO, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, 0x0, ARM_MPU_REGION_SIZE_256KB)
    },
    // executable and read only for both .text and .rodata (__data_start__ = 0044f800)
    // 4: 0x00440000, size=64K, XN=0, AP=0x6, TEX=0x1, S=0x1, C=0x1, B=0x1, SRD=0x80
    //    subregion_size=8K, subregions=0x7f
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    Y    0x00440000 0x00441fff <-- enabled
    //       1   0x02    Y    0x00442000 0x00443fff <-- enabled
    //       2   0x04    Y    0x00444000 0x00445fff <-- enabled
    //       3   0x08    Y    0x00446000 0x00447fff <-- enabled
    //       4   0x10    Y    0x00448000 0x00449fff <-- enabled
    //       5   0x20    Y    0x0044a000 0x0044bfff <-- enabled
    //       6   0x40    Y    0x0044c000 0x0044dfff <-- enabled
    //       7   0x80    N    0x0044e000 0x0044ffff
    {
        .RBAR = ARM_MPU_RBAR(4UL, 0x00440000UL),
        .RASR = ARM_MPU_RASR_EX(EXECUTE, ARM_MPU_AP_RO, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, 0x80, ARM_MPU_REGION_SIZE_64KB)
    },
    // executable and read only for both .text and .rodata (__data_start__ = 0044f800)
    // 5: 0x0044e000, size=8K, XN=0, AP=0x6, TEX=0x1, S=0x1, C=0x1, B=0x1, SRD=0xc0
    //    subregion_size=1K, subregions=0x3f
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    Y    0x0044e000 0x0044e3ff <-- enabled
    //       1   0x02    Y    0x0044e400 0x0044e7ff <-- enabled
    //       2   0x04    Y    0x0044e800 0x0044ebff <-- enabled
    //       3   0x08    Y    0x0044ec00 0x0044efff <-- enabled
    //       4   0x10    Y    0x0044f000 0x0044f3ff <-- enabled
    //       5   0x20    Y    0x0044f400 0x0044f7ff <-- enabled
    //       6   0x40    N    0x0044f800 0x0044fbff
    //       7   0x80    N    0x0044fc00 0x0044ffff
    {
        .RBAR = ARM_MPU_RBAR(5UL, 0x0044e000UL),
        .RASR = ARM_MPU_RASR_EX(EXECUTE, ARM_MPU_AP_RO, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, 0xc0, ARM_MPU_REGION_SIZE_8KB)
    },
    // stats and logging - write through (__logging_start__ = 00486800 .. __logging_end__=004f0000)
    // 6: 0x00480000, size=512K, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x1, B=0x0, SRD=0x81
    //    subregion_size=64K, subregions=0x7e
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    N    0x00480000 0x0048ffff
    //       1   0x02    Y    0x00490000 0x0049ffff <-- enabled
    //       2   0x04    Y    0x004a0000 0x004affff <-- enabled
    //       3   0x08    Y    0x004b0000 0x004bffff <-- enabled
    //       4   0x10    Y    0x004c0000 0x004cffff <-- enabled
    //       5   0x20    Y    0x004d0000 0x004dffff <-- enabled
    //       6   0x40    Y    0x004e0000 0x004effff <-- enabled
    //       7   0x80    N    0x004f0000 0x004fffff
    {
        .RBAR = ARM_MPU_RBAR(6UL, 0x00480000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, 0x81, ARM_MPU_REGION_SIZE_512KB)
    },
    // stats and logging - write through (__logging_start__ = 00486800 .. __logging_end__=004f0000)
    // 7: 0x00488000, size=32K, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x1, B=0x0, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(7UL, 0x00488000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, 0x0, ARM_MPU_REGION_SIZE_32KB)
    },
    // stats and logging - write through (__logging_start__ = 00486800 .. __logging_end__=004f0000)
    // 8: 0x00486000, size=8K, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x1, B=0x0, SRD=0x3
    //    subregion_size=1K, subregions=0xfc
    //    region mask enabled start      end
    //    ------ ---- ------- ---------- ----------
    //       0   0x01    N    0x00486000 0x004863ff
    //       1   0x02    N    0x00486400 0x004867ff
    //       2   0x04    Y    0x00486800 0x00486bff <-- enabled
    //       3   0x08    Y    0x00486c00 0x00486fff <-- enabled
    //       4   0x10    Y    0x00487000 0x004873ff <-- enabled
    //       5   0x20    Y    0x00487400 0x004877ff <-- enabled
    //       6   0x40    Y    0x00487800 0x00487bff <-- enabled
    //       7   0x80    Y    0x00487c00 0x00487fff <-- enabled
    {
        .RBAR = ARM_MPU_RBAR(8UL, 0x00486000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, 0x3, ARM_MPU_REGION_SIZE_8KB)
    },
    // after the inbox/outbox is mdx2_device_log_buffers_t which contains a description of where to find logging & stats and assert message, but we could just make it uncached, or manually flush
    // 9: 0x004f8000, size=32K, XN=1, AP=0x3, TEX=0x0, S=0x1, C=0x1, B=0x0, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(9UL, 0x004f8000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, 0x0, ARM_MPU_REGION_SIZE_32KB)
    },
    // inbox/outbox (hostmsg request/response) currently configured as uncached, but we should manually do the cache flush/invalidate.
    // 10: 0x004f8000, size=16K, XN=1, AP=0x3, TEX=0x1, S=0x1, C=0x0, B=0x0, SRD=0x0
    {
        .RBAR = ARM_MPU_RBAR(10UL, 0x004f8000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_UNCACHED, 0x0, ARM_MPU_REGION_SIZE_16KB)
    },
    // unused
    {
        .RBAR = ARM_MPU_RBAR(11UL, 0x00000000UL),
        .RASR = 0
    },
    // unused
    {
        .RBAR = ARM_MPU_RBAR(12UL, 0x00000000UL),
        .RASR = 0
    },
    // unused
    {
        .RBAR = ARM_MPU_RBAR(13UL, 0x00000000UL),
        .RASR = 0
    },
```
The yaml file describing the memory regions looks like this:

```yaml
#
# This is used by device.mk
#   after linking the firmware for the first time,
#   it runs run_update_memory_map.sh and creates $(BINDIR)/memory_map.h
#   which defines the MPU table that will be used for the dx2 build.
# 
# note:
#   DisableExec defaults to NEVER_EXECUTE
#   AccessPermission: defaults to ARM_MPU_AP_FULL
#

# to workaround ARM M7 errata:
# 1013783 PLD might perform linefill to address that would generate a MemManage Fault
# define MPU region 0 as 4G NO ACCESS.
region:
        comment:          start by defining all addresses as no access to avoid PLD errata.
        start_addr:       0x0
        end_addr:         0xffffffff
        AccessAttributes: NO_ACCESS
        AccessPermission: ARM_MPU_AP_NONE

region:
        comment:          DEV_CFG
        start_addr:       0x00f00000
        size:             33M
        AccessAttributes: DEVICE_SHAREABLE

region:
        comment:          OCR (default is shareable and works for LDREX/STREX, does it work for jtag?)
        start_addr:       0x400000
        size:             1MB
        AccessAttributes: WRITE_BACK_READ_AND_WRITE_ALLOCATE
region:
        comment:          after the inbox/outbox is mdx2_device_log_buffers_t which contains a description of where to find logging & stats and assert message, but we could just make it uncached, or manually flush
        start_addr:       0x004f8000
        size:             32K
        AccessAttributes: WRITE_THROUGH_NO_WRITE_ALLOCATE

region:
        comment:          inbox/outbox (hostmsg request/response) currently configured as uncached, but we should manually do the cache flush/invalidate.
        start_addr:       0x004f8000
        size:             16K
        AccessAttributes: UNCACHED

region:
        comment:          executable and read only for both .text and .rodata (__data_start__=$__data_start__)
        start_addr:       0x400000
        end_addr:         0x$__data_start__
        DisableExec:      EXECUTE
        AccessAttributes: WRITE_BACK_READ_AND_WRITE_ALLOCATE
        AccessPermission: ARM_MPU_AP_RO

region:
        comment:          stats and logging - write through (__logging_start__=$__logging_start__ .. __logging_end__=$__logging_end__)
        start_addr:       0x$__logging_start__
        end_addr:         0x$__logging_end__
        AccessAttributes: WRITE_THROUGH_NO_WRITE_ALLOCATE
```

The makefile looks like this,... it links a 'first_pass' firmware, figures out the end of .text, start of logging and of logging, then creates the MPU entries, recompiles the mpu table, and relinks the firmware:

```makefile
DEVICE_LIB=bunch of .o files

# first pass linking used to build mpu table
$(BINDIR)/device_first_pass: $(DEVICE_LIB)
	@echo " g++ linking device $@"
	@mkdir -p $(BINDIR)
	@$(LINKER) -o $@ $(filter %.o,$^)

# memory map is calculated after linking the firmware the first time.
$(BINDIR)/memory_map.h: $(BINDIR)/device_first_pass memory_map.yaml
	@echo ""
	@echo "--> extracting symbols"
	$(OBJDUMP) -t $(BINDIR)/device_first_pass > $(BINDIR)/device_first_pass.syms
	# note, the script sets the mpu table size to 15. This has to match the value of MPU_TABLE_SIZE
	# in mpu_table.h
	sh ../scripts/run_update_memory_map.sh \
	     $(BINDIR)/device_first_pass.syms \
	     memory_map.yaml $(BINDIR)/memory_map.h 15

# recompile the memory map with the results of the first firmware link.
# note the very very important -I$(BINDIR) at the beginning of the compile line to prefer our new memory_map.h file.
$(DEVICE_OBJDIR)/new_mpu_table.o: ../device/core/m7/mpu_table.cpp $(BINDIR)/memory_map.h
	@echo " compiling device $(notdir $<) (note with -I$(BINDIR) to use new memory_map.h)"
	@echo " if compiling new_mpu_table fails see run_update_memory_map.sh or contact Byron or Vas"
	@$(CC) -I$(BINDIR) $(CFLAGS) -o $@ -c $<

# create libdevice_final.a by removing the first pass mpu table from libdevice.a and substituting the new mpu_table
$(DEVICE_LIB_FINAL): $(DEVICE_LIB) $(DEVICE_OBJDIR)/new_mpu_table.o
	cp $(DEVICE_LIB) $(DEVICE_LIB_FINAL)
	$(ARCHIVE) d $(DEVICE_LIB_FINAL) mpu_table.o
	$(ARCHIVE) r $(DEVICE_LIB_FINAL) $(DEVICE_OBJDIR)/new_mpu_table.o

$(BINDIR)/device: $(DEVICE_OBJDIR)/device_main.o $(DEVICE_LIB_FINAL)
	@echo " linking device $@"
	@mkdir -p $(BINDIR)
	@$(LINK) $(LFLAGS) -o $@ \
		$(filter %.o,$^) \
		$(patsubst lib%.a,-l%,$(notdir $(filter %.a,$^)))
```

TEX B C abstractions
```c

#define TEX_111 7UL
#define TEX_110 6UL
#define TEX_101 5UL
#define TEX_000 0UL
#define TEX_001 1UL
#define TEX_010 2UL
#define C0 0UL
#define C1 1UL
#define S0 0UL
#define S1 1UL
#define B0 0UL
#define B1 1UL
#define SR_0 0x00UL
#define SR_11000000 0xc0UL // exclude top 2 of 8 subregions.
#define SR_00000011 0x03UL // exclude bottom 2 of 8 subregions.
#define SR_00000001 0x01UL // exclude bottom 1 of 8 subregions.
#define EXECUTE       0UL
#define EXECUTABLE    0UL
#define NEVER_EXECUTE 1UL

/*
the webpage here has the nicest picture i think:
https://www.st.com/resource/en/application_note/dm00272912-managing-memory-protection-unit-in-stm32-mcus-stmicroelectronics.pdf

Table 4. Cache properties and shareability
TEX C B Memory Type      | Description                         | Shareable
000 0 0 Strongly Ordered | Strongly Ordered                    | Yes
000 0 1 Device           | Shared Device                       | Yes
000 1 0 Normal           | Write through, no write allocate    | S bit
000 1 1 Normal           | Write-back, no write allocate       | S bit
001 0 0 Normal           | Non-cacheable                       | S bit
001 0 1 Reserved         | Reserved                            | Reserved
001 1 0 Undefined        | Undefined                           | Undefined
001 1 1 Normal           | Write-back, write and read allocate | S bit
010 0 0 Device           | Non-shareable device                | No
010 0 1 Reserved         | Reserved                            | Reserved

The subregion disable bits (SRD) flag whether a particular subregion is enabled or disabled. Disabling a
subregion means that another region overlapping the disabled range matches instead. If no other enabled
region overlaps the disabled subregion the MPU issues a fault.
For the products that implement a cache (only for STM32F7 Series and STM32H7 Series that implement L1-
cache) the additional memory attributes include:
* Cacheable/ non-cacheable: means that the dedicated region can be cached or not.
AN4838
Memory attributes
AN4838 - Rev 4 page 7/20
* Write through with no write allocate: on hits it writes to the cache and the main memory, on misses it
updates the block in the main memory not bringing that block to the cache.
* Write-back with no write allocate: on hits it writes to the cache setting dirty bit for the block, the main
memory is not updated. On misses it updates the block in the main memory not bringing that block to the
cache.
* Write-back with write and read allocate: on hits it writes to the cache setting dirty bit for the block, the
main memory is not updated. On misses it updates the block in the main memory and brings the block to the
cache.

*/

#define NO_ACCESS ARM_MPU_ACCESS_( TEX_000, S0, C0, B0 )

#define STRONGLY_ORDERED ARM_MPU_ACCESS_( TEX_000, S1, C0, B0 )

#define DEVICE_SHAREABLE ARM_MPU_ACCESS_( TEX_000, S1, C0, B1 )


// Write through with no write allocate: on hits it writes to the cache and the main memory, on misses it
// updates the block in the main memory not bringing that block to the cache.
//
// BYRON: this is a good strategy for the DBG_LOG data.
#define NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE ARM_MPU_ACCESS_( TEX_000, S1, C1, B0 )

// Write-back with no write allocate: on hits it writes to the cache setting dirty bit for the block, the main
// memory is not updated. On misses it updates the block in the main memory not bringing that block to the
// cache.
//
#define NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE ARM_MPU_ACCESS_( TEX_000, S1, C1, B1 )

// BYRON: question: when would you use NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE ?
// it's basically an optimization to NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, where if you're doing a write
// to the cache line you're thinking that maybe we'll never need to do a read.
// this would help with zero'ing a chunk of memory,... it wouldn't adversely affect the cache.
// or if you were placing data into a buffer that was about to DMA'd or used by another core,... (write only memory!)
// or memory that's written to, but won't be needed for quite a while.

// uncached.
#define NORMAL_UNCACHED ARM_MPU_ACCESS_( TEX_001, S1, C0, B0 )

// Write-back with write and read allocate: on hits it writes to the cache setting dirty bit for the block, the
// main memory is not updated. On misses it updates the block in the main memory and brings the block to the
// cache.
#define NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE ARM_MPU_ACCESS_( TEX_001, S1, C1, B1 )

#define NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE ARM_MPU_ACCESS_( TEX_001, S0, C1, B1 )

#define DEVICE_NON_SHAREABLE ARM_MPU_ACCESS_( TEX_010, S0, C0, B0 )
```
# TODO:

Add some pictures.

The code is still has dependencies on our build environment; Need to sanitize it further and create a github workflow for compiling/running/testing.

Add the freertos patch for the stack red zone
