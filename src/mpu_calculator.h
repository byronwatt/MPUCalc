/* copyright Microchip 2022, MIT License */
/**
* @file
* @brief
*   class for calculating mpu regions
*/

#ifndef MPU_CALCULATOR_H
#define MPU_CALCULATOR_H

#include <stdint.h>
#ifdef MDX2_FREERTOS_TARGET
#include "cpu_m7.h"
#endif
#include "mpu_armv7.h"

class mpu_calculator_t {
public:
    // pretending there are more entries available so that we can
    // use the tools to visualize which regions are using more regions.
    static const uint32_t MAX_ENTRIES = 20;
    uint32_t subregions_mask;
    uint32_t first_addr;
    uint32_t next_addr;
    uint32_t mpu_region_number;
    uint32_t region_start_addr;
    uint32_t start_addr;
    uint32_t end_addr;
    uint32_t num_entries;
    uint32_t num_subregions;
    uint32_t region_size;
    uint32_t actual_mpu_size;
    ARM_MPU_Region_t mpu_table[MAX_ENTRIES];

    mpu_calculator_t():mpu_region_number(),num_entries() {}

    void try_subregion_size( uint32_t subregion_size, uint32_t region_start_addr_ );

    void select_best_mpu_size();

    void find_size_and_subregions();

    inline uint32_t region_size_to_region_bits( uint32_t size );

    void build_entry( uint32_t DisableExec, uint32_t AccessPermission, uint32_t AccessAttributes);

    bool build_best_mpu_entries( uint32_t start_addr_, uint32_t end_addr_, uint32_t DisableExec, uint32_t AccessPermission, uint32_t AccessAttributes);

    bool copy_child_entries( mpu_calculator_t &child );
};


inline uint32_t mpu_calculator_t::region_size_to_region_bits( uint32_t size )
{
    // e.g. 32 == 4
    // #define ARM_MPU_REGION_SIZE_32B      ((uint8_t)0x04U) ///!< MPU Region Size 32 Bytes
    return __builtin_ctz(size)-1;
}

#endif