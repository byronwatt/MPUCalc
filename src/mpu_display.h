/* copyright Microchip 2022, MIT License */
/**
* @file
* @brief
*   class for displaying a memory map
*/

#ifndef MPU_DISPLAY_H
#define MPU_DISPLAY_H

#include <stdio.h>
#include <stdint.h>
#ifdef MDX2_FREERTOS_TARGET
#include "cpu_m7.h"
#endif
#include "mpu_armv7.h"
#ifndef MDX2_SMALL_MEMORY
#include <string>
#endif

// failed attempt to print a nice memory map,... maybe could be done as a debug_cmd or from the host,...
// kinda hard to figure out the end of the region unless you simply scan forward 32 bytes at a time.
class mpu_entry_t {
public:
    uint32_t mpu_RBAR;
    uint32_t mpu_RASR;

    // RBAR fields
    uint32_t BaseAddress;
    uint32_t Region;

    // RASR fields
    uint32_t TypeExtField;
    uint32_t IsShareable;
    uint32_t IsCacheable;
    uint32_t IsBufferable;
    uint32_t DisableExec;
    uint32_t AccessPermission;
    uint32_t SubRegionDisable;
    uint32_t Size;
    uint32_t AccessAttributes;
    uint32_t enable;

    uint32_t size_pwr_2; // 0 means 2^32
    uint32_t subregion_size;
#ifndef MDX2_SMALL_MEMORY
    std::string comment;
#endif
    void set( uint32_t RBAR, uint32_t RASR );
    void print( FILE *f, const char *prefix );
    bool region_active(uint32_t subregion_number)
    {
        if (enable && !(SubRegionDisable & (1<<subregion_number)))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    const char *access_type_to_string() const;
    const char *access_type_to_code() const;
    const char *disable_exec_to_code() const;
    const char *access_permission_to_code() const;
};

class mpu_display_t {
public:
    // pretending there are more entries available so that we can
    // use the tools to visualize which regions are using more regions.
    static const uint32_t MAX_ENTRIES = 20;
    ARM_MPU_Region_t mpu_table[MAX_ENTRIES];
    void get_first_and_last_address(uint32_t *first_addr_ptr, uint32_t *last_addr_ptr);
    void display_memory_map( FILE *f, const char *prefix );
    void display_entries( FILE *f, const char *prefix );
    void set(uint32_t i,uint32_t RBAR,uint32_t RASR)
    {
        mpu_table[i].RBAR=RBAR;
        mpu_table[i].RASR=RASR;
    }
#ifndef MDX2_SMALL_MEMORY
    void set(uint32_t i,uint32_t RBAR,uint32_t RASR, std::string comment)
    {
        mpu_table[i].RBAR=RBAR;
        mpu_table[i].RASR=RASR;
        mpu_entries[i].comment = comment;
    }
#endif
    mpu_display_t():mpu_table(),mpu_entries(){}
private:
    mpu_entry_t mpu_entries[MAX_ENTRIES];
} ;

#endif