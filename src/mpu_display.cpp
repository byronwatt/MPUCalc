/* copyright Microchip 2022, MIT License */
/**
* @file
* @brief
*   class for calculating mpu regions
*/

#include "mpu_display.h"
#include "range_vector.h"
#include "mpu_armv7.h"
#include "configure_mpu.h"


/// return string describing the memory region (a combination of three of the arguments to the CMSIS ARM_MPU_RASR_EX() macro)
const char *mpu_entry_t::access_type_to_string() const
{
    if (DisableExec == 0)
    {
        switch (AccessAttributes)
        {
            case NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE:
            {
                if (AccessPermission == ARM_MPU_AP_RO)
                {
                    return "WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)";
                }
                else
                {
                    // not currently used, but we could if we needed to save MPU entries.
                    return "WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached, execute allowed)";
                }
            }
        }
        return "unknown TEX/C/S/B/AP combination with execute allowed";
    }
    if (AccessPermission == ARM_MPU_AP_RO)
    {
        switch (AccessAttributes)
        {
            case NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE:
            {
                return "WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)";
            }
        }
        return "unknown TEX/C/S/B/AP combination with read-only access";
    }
    else if (AccessPermission == ARM_MPU_AP_NONE)
    {
        switch (AccessAttributes)
        {
            case NO_ACCESS:
                return "NO_ACCESS";
        }
        return "unknown TEX/C/S/B/AP combination with no access";
    }
    switch (AccessAttributes)
    {
        case STRONGLY_ORDERED:
            return "STRONGLY_ORDERED";
        case DEVICE_SHAREABLE:
            return "DEVICE_SHAREABLE";
        case NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE:
            return "WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)";
        case NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE:
            return "WRITE_BACK_NO_WRITE_ALLOCATE (logging & stats)";
        case NORMAL_UNCACHED:
            return "UNCACHED e.g. inbox/outbox, pktmem";
        case NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE:
            return "WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)";
        case NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE:
            return "WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE (fully cached and works with jtag)";
        case DEVICE_NON_SHAREABLE:
            return "DEVICE_NON_SHAREABLE";
    }
    return "unknown TEX/C/S/B/AP combination";
}

/// return string used with the CMSIS ARM_MPU_RASR_EX() macro
const char *mpu_entry_t::access_type_to_code() const
{
    switch (AccessAttributes)
    {
        case NO_ACCESS:
            return "NO_ACCESS";
        case STRONGLY_ORDERED:
            return "STRONGLY_ORDERED";
        case DEVICE_SHAREABLE:
            return "DEVICE_SHAREABLE";
        case NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE:
            return "NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE";
        case NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE:
            return "NORMAL_WRITE_BACK_NO_WRITE_ALLOCATE";
        case NORMAL_UNCACHED:
            return "NORMAL_UNCACHED";
        case NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE:
            return "NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE";
        case NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE:
            return "NORMAL_WRITE_BACK_READ_AND_WRITE_ALLOCATE_NON_SHAREABLE";
        case DEVICE_NON_SHAREABLE:
            return "DEVICE_NON_SHAREABLE";
    }
    return "unknown TEX/C/S/B combination";
}

/// return string used with the CMSIS ARM_MPU_RASR_EX() macro
const char *mpu_entry_t::disable_exec_to_code() const
{
    if (DisableExec == 0)
    {
        return "EXECUTE";
    }
    return "NEVER_EXECUTE";
}

/// return string used with the CMSIS ARM_MPU_RASR_EX() macro
const char *mpu_entry_t::access_permission_to_code() const
{
    switch (AccessPermission)
    {
        case ARM_MPU_AP_RO:
            return "ARM_MPU_AP_RO";
        case ARM_MPU_AP_FULL:
            return "ARM_MPU_AP_FULL";
        case ARM_MPU_AP_NONE:
            return "ARM_MPU_AP_NONE";
    }
    return "unknown AccessPermission";
}


// set the mpu entry and extract the various fields
void mpu_entry_t::set( uint32_t RBAR, uint32_t RASR )
{
    mpu_RBAR = RBAR;
    mpu_RASR = RASR;

    BaseAddress = mpu_RBAR & MPU_RBAR_ADDR_Msk;
    Region      = mpu_RBAR & MPU_RBAR_REGION_Msk;

    TypeExtField     = (mpu_RASR & MPU_RASR_TEX_Msk)  >> MPU_RASR_TEX_Pos;
    IsShareable      = (mpu_RASR & MPU_RASR_S_Msk)    >> MPU_RASR_S_Pos;
    IsCacheable      = (mpu_RASR & MPU_RASR_C_Msk)    >> MPU_RASR_C_Pos;
    IsBufferable     = (mpu_RASR & MPU_RASR_B_Msk)    >> MPU_RASR_B_Pos;
    DisableExec      = (mpu_RASR & MPU_RASR_XN_Msk)   >> MPU_RASR_XN_Pos;
    AccessPermission = (mpu_RASR & MPU_RASR_AP_Msk)   >> MPU_RASR_AP_Pos;
    SubRegionDisable = (mpu_RASR & MPU_RASR_SRD_Msk)  >> MPU_RASR_SRD_Pos;
    Size             = (mpu_RASR & MPU_RASR_SIZE_Msk) >> MPU_RASR_SIZE_Pos;
    // note AccessAttributes is a combination of tex,s,c&b that is passed to the CMSIS ARM_MPU_ACCESS_() macro
    AccessAttributes = (mpu_RASR & (MPU_RASR_TEX_Msk | MPU_RASR_S_Msk | MPU_RASR_C_Msk | MPU_RASR_B_Msk));
    enable           = (mpu_RASR & MPU_RASR_ENABLE_Msk);

    size_pwr_2 = 1 << (Size +1);
    subregion_size = size_pwr_2/8;
    if (Size == 31)
    {
        subregion_size = 0x20000000;
        size_pwr_2 = 0;
    }
    //if (enable)
    //{
    //    fprintf(f,"0x%08x .. 0x%08x: %d 0x%02x %s\n",BaseAddress,BaseAddress+size_pwr_2-1,Region,SubRegionDisable,access_type_to_string());
    //}
}

/**
 * @brief
 *    print (numerator/denominator) with one decimal place if needed
 * e.g.
 *    1246, 1000
 * is displayed as:
 *    1.3
 * e.g.
 *    ((numerator + denominator/20) * 10)/1000
 */
static void format_fraction_1_dp( char buffer[10], uint32_t numerator, uint32_t denominator )
{
    // check that (numerator+denominator/20)*10 will not overflow.
    if (numerator < UINT32_MAX/10-denominator/20)
    {
        uint32_t fixed_point = ((numerator + denominator/20)) * 10 / denominator;
        sprintf(buffer,"%d.%01d",fixed_point/10,fixed_point%10);
    }
    else
    {
        // if (numerator+denominator/20)*10 will overflow, divide numerator and denominator by a bit first.
        format_fraction_1_dp( buffer, (numerator + 512) / 1024, denominator / 1024 );
    }
    // strip trailing '0's
    char *tail = &buffer[strlen(buffer)-1];
    while (*tail == '0')
    {
        *tail = 0;
        tail--;
    }
    // strip trailing '.'
    if (*tail == '.')
    {
        *tail = 0;
    }
}
static void format_size( char buffer[10], uint32_t size_in_bytes )
{
    const char *units;
    if (size_in_bytes >= 1024*1024*1024)
    {
        format_fraction_1_dp(buffer,size_in_bytes,1024*1024*1024);
        units = "G";
    }
    else if (size_in_bytes >= 1024*1024)
    {
        format_fraction_1_dp(buffer,size_in_bytes,(1024*1024));
        units = "M";
    }
    else if (size_in_bytes >= 1024)
    {
        format_fraction_1_dp(buffer,size_in_bytes,1024);
        units = "K";
    }
    else
    {
        sprintf(buffer,"%d",size_in_bytes);
        units = "";
    }
    strcat(buffer,units);
}

void mpu_entry_t::print(FILE *f, const char *prefix)
{
    if (enable)
    {
        fprintf(f,"%s%s\n",prefix,comment.c_str());
        fprintf(f,"%s%d: 0x%08x",prefix,Region,BaseAddress);
        char region_buffer[10];
        if (size_pwr_2 == 0)
        {
            sprintf(region_buffer,"4G");
        }
        else
        {
            format_size(region_buffer,size_pwr_2);
        }
        fprintf(f,", size=%s",region_buffer);
        fprintf(f,", XN=%d, AP=0x%x, TEX=0x%x, S=0x%x, C=0x%x, B=0x%x, SRD=0x%x\n",
            DisableExec,
            AccessPermission,
            TypeExtField,
            IsShareable,
            IsCacheable,
            IsBufferable,
            SubRegionDisable );
        if (SubRegionDisable != 0)
        {
            char buffer[10];
            format_size(buffer,subregion_size);
            fprintf(f,"%s   subregion_size=%s",prefix,buffer);
            uint32_t subregions = (~SubRegionDisable) & 0xff;
            fprintf(f,", subregions=0x%02x\n",subregions);
            // 8 subregions that can be either active or inactive
            if (subregions != 0)
            {
                fprintf(f,"%s   region mask enabled start      end\n",prefix);
                fprintf(f,"%s   ------ ---- ------- ---------- ----------\n",prefix);

                for (uint32_t i = 0; i < 8; i++)
                {
                    uint32_t subregion_start = BaseAddress + subregion_size*i;
                    uint32_t subregion_end = BaseAddress + subregion_size*(i+1)-1;
                    // if enabled
                    if (subregions&(0x1<<i))
                    {
                        fprintf(f,"%s      %d   0x%02x    Y    0x%08x 0x%08x <-- enabled\n", prefix, i, 0x1<<i, subregion_start, subregion_end);
                    }
                    else // disabled
                    {
                        fprintf(f,"%s      %d   0x%02x    N    0x%08x 0x%08x\n", prefix, i, 0x1<<i, subregion_start, subregion_end);
                    }
                }
            }
        }
        // now display it as a RBAR/RASR for easy copy paste into code:
        //    {
        //        .RBAR = ARM_MPU_RBAR(3UL, 0x007f8000UL),
        //        .RASR = ARM_MPU_RASR_EX(NEVER_EXECUTE, ARM_MPU_AP_FULL, NORMAL_WRITE_THROUGH_NO_WRITE_ALLOCATE, SR_0, ARM_MPU_REGION_SIZE_32KB)
        //    },
        fprintf(f,"    {\n");
        fprintf(f,"        .RBAR = ARM_MPU_RBAR(%dUL, 0x%08xUL),\n",Region,BaseAddress);
        fprintf(f,"        .RASR = ARM_MPU_RASR_EX(%s, %s, %s, 0x%x, ARM_MPU_REGION_SIZE_%sB)\n",
                disable_exec_to_code(),access_permission_to_code(),access_type_to_code(),SubRegionDisable,region_buffer);
        fprintf(f,"    },\n");
    }
    else // disabled
    {
        if (Region != 0)
        {
            fprintf(f,"%s%s\n",prefix,comment.c_str());
            fprintf(f,"    {\n");
            fprintf(f,"        .RBAR = ARM_MPU_RBAR(%dUL, 0x%08xUL),\n",Region,BaseAddress);
            fprintf(f,"        .RASR = 0\n");
            fprintf(f,"    },\n");
        }
    }
}

static void display_interval( const DisjointInterval<uint32_t,mpu_entry_t*>& rl, FILE *f, const char *prefix )
{
    uint32_t size = rl.stop-rl.start+1;
    char size_string[10];
    format_size(size_string,size);
    if (rl._list.empty())
    {
        fprintf(f,"%s%08x %08x %6s  . unmapped\n",prefix,rl.start,rl.stop,size_string);
    }
    else
    {
        uint32_t max_region = 0;
        mpu_entry_t *max_entry;
        for( typename std::list< RangeValue<uint32_t,mpu_entry_t*> >::const_iterator i=rl._list.begin();
             i != rl._list.end();
             i++)
        {
            RangeValue<uint32_t,mpu_entry_t*> rv = *i;
            mpu_entry_t *e = rv.value;
            if (e->Region >= max_region)
                max_entry = e;
        }
        fprintf(f,"%s%08x %08x %6s %2d %s\n",prefix,rl.start,rl.stop,size_string,max_entry->Region,max_entry->access_type_to_string());
    }
}
static void display_range( const  DisjointRangeVector<uint32_t,mpu_entry_t*>& arv, FILE *f, const char *prefix ) {
    for (typename std::vector< DisjointInterval<uint32_t,mpu_entry_t*> >::const_iterator i = arv._vector_of_disjoint_intervals.begin();
        i != arv._vector_of_disjoint_intervals.end();
        i++)
    {
        display_interval( *i, f, prefix );
    }
}

/*
 * display a memory map like:
 *
 * start    end      size   #  description
 * -------- -------- ------ -- -----------
 * 00000000 003fffff     4M  . unmapped
 * 00400000 00437fff   224K  7 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)
 * 00438000 0043b7ff    14K  8 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)
 * 0043b800 0046ebff   205K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)
 * 0046ec00 0046ffff     5K 13 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
 * 00470000 0047ffff    64K 12 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
 * 00480000 004effff   448K 11 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
 * 004f0000 004f7fff    32K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)
 * 004f8000 004fbfff    16K  4 UNCACHED (inbox/outbox)
 * 004fc000 004ffbff    15K  3 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
 * 004ffc00 004fffff     1K  5 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)
 * 00500000 00efffff    10M  . unmapped
 * 00f00000 00ffffff     1M  0 DEVICE_SHAREABLE
 * 01000000 02ffffff    32M  1 DEVICE_SHAREABLE
 * 03000000 dfffffff   3.4G  . unmapped
 * e0000000 e000ffff    64K  6 STRONGLY_ORDERED
 * e0010000 fffffffc 511.9M  . unmapped
 *
 * see mpu_calculator_test.cpp for example usage,
 *
 * also called by mpu_dump() in configure_mpu.cpp
 * 
 * TODO: refactoring suggestion: first convert from a DisjointRangeVector<> to a normal vector<>
 *  then print or return the vector<> for easier unit tests or custom formatting.  (currently the formatting and algorithm are tightly coupled)
 */
void mpu_display_t::display_memory_map(FILE *f, const char *prefix)
{
    for (uint32_t i=0;i<MAX_ENTRIES;i++)
    {
        mpu_entry_t *e = &mpu_entries[i];
        e->set(mpu_table[i].RBAR,mpu_table[i].RASR);
    }

    DisjointRangeVector<uint32_t,mpu_entry_t*>::range_vector v;
    for (uint32_t i=0;i<MAX_ENTRIES;i++)
    {
        mpu_entry_t *e = &mpu_entries[i];
        if (e->enable)
        {
            uint32_t subregions = (~e->SubRegionDisable) & 0xff;
            while (subregions != 0)
            {
                uint32_t first_subregion = __builtin_ctz(subregions);
                uint32_t next_subregion = first_subregion;
                while (subregions & (1<<(next_subregion)))
                {
                    subregions &= ~(1<<next_subregion);
                    next_subregion++;
                }
                //uint32_t last_subregion = next_subregion-1;
                uint32_t start_addr = e->BaseAddress + e->subregion_size*first_subregion;
                uint32_t end_addr = e->BaseAddress + e->subregion_size*next_subregion-1;
                DisjointRangeVector<uint32_t,mpu_entry_t*>::range_value RangeValue(start_addr,end_addr,e);
                v.push_back(RangeValue);
            }
        }
    }

    // set the minimum and maximum values to 0,0xffffffff
    // and initialize the disjoint vector to 'v'
    // this converts the vector of overlapping ranges
    // into a disjoint set of intervals
    DisjointRangeVector<uint32_t,mpu_entry_t*> rv(0,0xffffffff,&v);

    fprintf(f,"%sstart    end      size   #  description\n",prefix);
    fprintf(f,"%s-------- -------- ------ -- -----------\n",prefix);

    // display the disjoint vector.
    ::display_range( rv, f, prefix );
    fprintf(f,"\n");

}

/*
 * extract first and last address (used for unit tests)
 */
void mpu_display_t::get_first_and_last_address(uint32_t *first_addr_ptr, uint32_t *last_addr_ptr)
{
    uint32_t first_addr = 0xffffffff;
    uint32_t last_addr = 0;
    for (uint32_t i=0;i<MAX_ENTRIES;i++)
    {
        mpu_entry_t *e = &mpu_entries[i];
        e->set(mpu_table[i].RBAR,mpu_table[i].RASR);
    }

    for (uint32_t i=0;i<MAX_ENTRIES;i++)
    {
        mpu_entry_t *e = &mpu_entries[i];
        if (e->enable)
        {
            uint32_t subregions = (~e->SubRegionDisable) & 0xff;
            while (subregions != 0)
            {
                uint32_t first_subregion = __builtin_ctz(subregions);
                uint32_t next_subregion = first_subregion;
                while (subregions & (1<<(next_subregion)))
                {
                    subregions &= ~(1<<next_subregion);
                    next_subregion++;
                }
                //uint32_t last_subregion = next_subregion-1;
                uint32_t start_addr = e->BaseAddress + e->subregion_size*first_subregion;
                uint32_t end_addr = e->BaseAddress + e->subregion_size*next_subregion-1;
                if (start_addr < first_addr)
                {
                    first_addr = start_addr;
                }
                if (end_addr > last_addr)
                {
                    last_addr = end_addr;
                }
            }
        }
    }
    *first_addr_ptr = first_addr;
    *last_addr_ptr = last_addr;
}


/*
 * display a memory map like:
 *
 * start    end      size   #  description
 * -------- -------- ------ -- -----------
 * 00000000 003fffff     4M  . unmapped
 * 00400000 00437fff   224K  7 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)
 * 00438000 0043b7ff    14K  8 WRITE_BACK_READ_AND_WRITE_ALLOCATE (read-only, execute allowed)
 * 0043b800 0046ebff   205K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)
 * 0046ec00 0046ffff     5K 13 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
 * 00470000 0047ffff    64K 12 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
 * 00480000 004effff   448K 11 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
 * 004f0000 004f7fff    32K  2 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)
 * 004f8000 004fbfff    16K  4 UNCACHED (inbox/outbox)
 * 004fc000 004ffbff    15K  3 WRITE_THROUGH_NO_WRITE_ALLOCATE (logging)
 * 004ffc00 004fffff     1K  5 WRITE_BACK_READ_AND_WRITE_ALLOCATE (fully cached)
 * 00500000 00efffff    10M  . unmapped
 * 00f00000 00ffffff     1M  0 DEVICE_SHAREABLE
 * 01000000 02ffffff    32M  1 DEVICE_SHAREABLE
 * 03000000 dfffffff   3.4G  . unmapped
 * e0000000 e000ffff    64K  6 STRONGLY_ORDERED
 * e0010000 fffffffc 511.9M  . unmapped
 *
 * see mpu_calculator_test.cpp for example usage,
 *
 * also called by mpu_dump() in configure_mpu.cpp
 * 
 */
void mpu_display_t::display_entries(FILE *f, const char *prefix)
{
    for (uint32_t i=0;i<MAX_ENTRIES;i++)
    {
        mpu_entry_t *e = &mpu_entries[i];
        e->set(mpu_table[i].RBAR,mpu_table[i].RASR);
        e->print(f,prefix);
    }
}

