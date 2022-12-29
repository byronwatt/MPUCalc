#ifndef CPU_M7_H
#define CPU_M7_H

/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __CM7_REV                 0x0000U   /* Core revision r0p0 */
#define __MPU_PRESENT             1U        /* MPU present */
#define __VTOR_PRESENT            1U        /* VTOR present */
#define __NVIC_PRIO_BITS          8U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             0U        /* no FPU present */
#define __FPU_DP                  0U        /* unused */
#define __ICACHE_PRESENT          1U
#define __DCACHE_PRESENT          1U
#define __DTCM_PRESENT            1U

//#include "mdx2_multitask_irq.h"
#include "core_cm7.h"                       /* Processor and core peripherals */
//#include "system_ARMCM7.h"                  /* System Header */

extern "C" void SystemInit(void);
extern "C" void cache_enable(void);
extern "C" void cache_invalidate(void);
extern "C" void fpu_enable(void);


static inline uint32_t arm_cpu_intr_disable()
{
    uint32_t primask = __get_PRIMASK();

    /* disable all interrupts of priority 0 and higher? */
    __set_PRIMASK(0);

    //__asm volatile ( "DSB" );
    //__asm volatile ( "ISB" );
    return primask;
}

static inline void arm_cpu_intr_restore(uint32_t primask)
{
        __set_PRIMASK(primask);
}

/** 
 * @brief
 * cpu_interrupt_disable_guard is a class to disable interrupts for the life of the object.
 *      
 *  Instead of code that explicitly disables and restores interrupts like:
 *
 *      void func()
 *      {
 *         mask = arm_cpu_intr_disable();
 *         if (a) {
 *            arm_cpu_intr_restore(mask);
 *            return;
 *         }
 *         ...
 *         arm_cpu_intr_restore(mask);
 *      }
 *
 *  You can instead let the cpu_interrupt_disable_guard constructor/destructor handle disabling/restoring interrupts.
 *  (this strategy is called RAII resource allocation is initialization)
 *
 *      void func()
 *      {
 *         cpu_interrupt_disable_guard atomic_guard();
 *         if (a) {
 *            return; // interrupts re-enabled here.
 *         }
 *         ...
 *         // interrupts re-enabled here.
 *      }
 *
 * note: you can examine the assembly language by looking at DEBUG_COMMAND_ACTION(atomic_guard) in dev_module_debug_cmds.cpp:
 *
 *     ./scripts/run_show_arm_assembly.sh debug_command_atomic_guard::action
 *     debug_command_atomic_guard::action
 *         0x1462a0b0:    e10f0000    ....    MRS      r0,APSR ; formerly CPSR
 *         0x1462a0b4:    e38010c0    ....    ORR      r1,r0,#0xc0
 *         0x1462a0b8:    e129f001    ..).    MSR      CPSR_cf,r1
 *         0x1462a0bc:    f57ff04f    O...    DSB      
 *         0x1462a0c0:    f57ff06f    o...    ISB      
 *         0x1462a0c4:    e129f000    ..).    MSR      CPSR_cf,r0
 *         0x1462a0c8:    e12fff1e    ../.    BX       lr
 *
 * you can measure the performance with:
 *     $ ./mdx2_util cmd: atomic_guard iterations=1000
 *     sending: "atomic_guard iterations=1000"
 *     atomic_guard iterations=1000
 *      (1000 iterations)
 *      took 47225 nanoseconds
 *      (47 microseconds)
 *      (0.047 microseconds per iteration)
 *
 * e.g. 36 nanoseconds (the base no-op loop takes 11 nanoseconds so the disable/restore takes an additional 36 nanoseconds)
 */
class cpu_interrupt_disable_guard {
public:
    inline cpu_interrupt_disable_guard();
    inline ~cpu_interrupt_disable_guard();
private:
    uint32_t primask; ///< value of cpsr register to restore.
};

/** 
* @brief 
*   constructor disables interrupts
*/
cpu_interrupt_disable_guard::cpu_interrupt_disable_guard()
{
    primask = arm_cpu_intr_disable();
}

/** 
* @brief 
*   deconstructor restores interrupts to the value prior to calling arm_cpu_intr_disable.
*/
inline cpu_interrupt_disable_guard::~cpu_interrupt_disable_guard()
{
    arm_cpu_intr_restore(primask);
}

/// trick to prevent someone from accidentally using cpu_interrupt_disable_guard as a temporary object
/// https://stackoverflow.com/questions/16189742/how-to-avoid-c-anonymous-objects
#define cpu_interrupt_disable_guard(x) cpu_interrupt_disable_guard needs to be used with a variable name!?!?!

/* --------  End of section using anonymous unions and disabling warnings  -------- */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


#endif

