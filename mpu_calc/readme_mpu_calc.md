## overview
Calculate MPU entries after the linker has run.

## the 2 pass linking done in device.mk

this is how device.mk links the firmware first to figure out the section sizes,
then calculates the mpu table, compiles one file and relinks the firmware a second and final time.

1.    create a first cut of the device.
  ==>  builds/freertos_linarogcc_reva/bin/mdx2l_device_first_pass

2.  extract the various ends of various sections. (__data_start__, __logging_start__, __logging_end__)
```bash
  grep -E '__data_start__|__logging_end__|__logging_start__' builds/lab/mdx2l_device.syms

  004f0000 g       .ctxt_space    00000000 __logging_end__
  00486800 g       .logging   00000000 __logging_start__
  0044f800 g       .data  00000000 __data_start__
  grep -E '__data_start__|__logging_end__|__logging_start__' builds/lab/mdx2l_device.syms \
   | sed -r 's%^([0-9a-f]*)\s.*\s(__.*)$%s/${\2}/\1/;%' > search_and_replace.pl
cp ../device/core/m7/unit_test/memory_map_mdx2l.yaml memory_map.yaml
```
3. based on these 'variables' and the other fixed stuff create the full mpu table.
  run mpu_calc to create mpu_entries.h

4. build a new version with the new memory map (almost like build twice but that would be bad.)
  does not overwrite memory_map.h,... create a new memory_map.h
  and compiles it to a different spot.

5. link everything again,... but with the new memory_map.o file.
 rebuild memory_map.cpp to builds/freertos_linarogcc_reva/bin/mdx2l_device_first_pass
 the makefile will build the device to libdevice.a

## makefile rules:

## normal device build but change it to 'first_pass'
```make
$(BINDIR)/mdx2l_device_first_pass.elf: libdevice.a
     link...
```
## create the updated memory map either in the object folder or the generated folder
## don't want to confusing people and check this in every time (this would be a source of conflicts)
```make
$(OBJDIR)/memory_map.h: $(BINDIR)/mdx2l_device_first_pass
  look for __data_start__, __logging_start__, __logging_end__ in mdx2l_device_first_pass.elf
    patch memory_map.yaml with these values maybe with envsubst
    run mpu_calc -o $@
 
$(OBJDIR)/final_memory_map.o: device/core/memory_map.cpp $(OBJDIR)/memory_map.h
     gcc device/core/memory_map.cpp -o $@ -I $(OBJDIR)

# final device build but change it to 'first_pass'
$(BINDIR)/mdx2l_device.elf: libdevice.a $(OBJDIR)/final_memory_map.o
     link -ldevice $(OBJDIR)/final_memory_map.o $@
```

sample yaml file:

 this program takes a yaml file like:
```yaml
 region:
    start: 0x0
    size:  1M
    attributes: 
    comment: 
 region:
    start: 0x0
    size:  1M
    attributes: 
    comment: 
 region:
    start: 0x0
    size:  1M
    attributes: 
    comment: 
```
 and creates the corresponding header file.


## problem

hopefully we get something like this out:

```c++
static const ARM_MPU_Region_t mpu_settings[] = {

    // 2. configure 0x0040_0000 .. 0x004f_ffff as fully cached. 
    {
        .RBAR = ARM_MPU_RBAR(2UL, 0x00400000UL),
        // note: change to EXECUTABLE if you comment out Configure_ReadOnlyMemory_for_text_and_rodata()
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE, SR_0, ARM_MPU_REGION_SIZE_1MB)
    },


    // 3. configure 0x004f_8000 .. 0x004f_ffff as write-through no write allocate
    // after the inbox/outbox is mdx2_device_log_buffers_t, which contains a description of 
    // where to find logging, stats and console as well as the assert message buffer.
    {
        .RBAR = ARM_MPU_RBAR(3UL, 0x004f8000UL),
        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, SR_0, ARM_MPU_REGION_SIZE_32KB)
    },

};
```
