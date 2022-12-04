/* copyright Microchip 2022, MIT License */
/*
 * this file is compiled twice.
 * 
 * the first time uses a memory_map.h from device/core/m7
 *
 * the second time is recompiled with -I builds/freertos_reva/obj/device which has a tweaked memory_map.h file that matches the 
 * memory sizes found from the first pass linker.
 *
 */
#include "cpu_m7.h"
#include "mpu_armv7.h"
#include "configure_mpu.h"
#include "mpu_table.h"


/**
 *  special section for mpu data so that changing the order of linking doesn't affect the layout of the executable
 *  note: exactly 16 entries so that an overflow becomes a compiler error
 */
__attribute__((section(".mpu_table"))) const ARM_MPU_Region_t mpuTable[MPU_TABLE_SIZE] = {
#include "memory_map.h"
} ;
