/* copyright Microchip 2022, MIT License */
/**
* @file
* @brief
*  header file for configuring the arm m7 mpu
*/

#ifndef CONFIGURE_MPU_H
#define CONFIGURE_MPU_H

#include "mpu_armv7.h"

extern "C" void mpu_dump();

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
//
// perhaps the only memory that should be WRITE_ALLOCATE is stack,... unfortunately stack is part of heap.


// uncached.
#define NORMAL_UNCACHED ARM_MPU_ACCESS_( TEX_001, S1, C0, B0 )

// Write-back with write and read allocate: on hits it writes to the cache setting dirty bit for the block, the
// main memory is not updated. On misses it updates the block in the main memory and brings the block to the
// cache.
#define NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE ARM_MPU_ACCESS_( TEX_001, S1, C1, B1 )

#define NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE ARM_MPU_ACCESS_( TEX_001, S0, C1, B1 )

#define DEVICE_NON_SHAREABLE ARM_MPU_ACCESS_( TEX_010, S0, C0, B0 )

void Configure_MPU();

void mpu_configure_region( uint32_t base_address, uint32_t size_in_bytes, uint32_t DisableExec, uint32_t AccessPermission, uint32_t AccessAttributes );

void mpu_clear_region();

#ifdef MDX2_FREERTOS_TARGET
#endif

#endif // CONFIGURE_MPU_H