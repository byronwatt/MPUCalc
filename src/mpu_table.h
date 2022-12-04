/* copyright Microchip 2022, MIT License */

/**
 *  special section for mpu data so that changing the order of linking doesn't affect the layout of the executable
 *  note: exactly 16 entries so that an overflow becomes a compiler error
 */

/// IF UPDATING MPU_TABLE_SIZE, YOU MUST UPDATE run_update_memory_map.sh in DEVICE.MK 
#define MPU_TABLE_SIZE 15 

__attribute__((section(".mpu_table"))) extern const ARM_MPU_Region_t mpuTable[MPU_TABLE_SIZE];

