#ifndef ATOMIC_INTRINSICS_H
#define ATOMIC_INTRINSICS_H

#include "mdx2_porting.h" // for MDX2_LIKELY

/**
 * @brief
 *   atomic_LL calls LDREX and returns the value.
 * borrowed from:
 *   https://stackoverflow.com/users/2512648/alexandre-pereira-nunes
 *   https://stackoverflow.com/questions/5745880/simulating-ldrex-strex-load-store-exclusive-in-cortex-m0
 *
 * @param[in] ptr
 *
 * @return value returned by LDREX
 */
static inline uint32_t atomic_LL(volatile uint32_t *ptr)
{
  uint32_t value;
  __asm__ __volatile__("ldrex %0, [%1]" : "=r" (value) : "r" (ptr));
  return value;
}

/**
 * @brief
 *   atomic_SC calls STREX result,value,ptr and returns the result (0 == success, 1 == failure).
 *
 * @param[in] ptr
 * @param[in] value
 *
 * @return completion code returned by STREX
 */
static inline uint32_t atomic_SC(volatile uint32_t *ptr, uint32_t value)
{
  uint32_t strex_result;
  __asm__ __volatile__("strex %0, %2, [%1]" :
          "=&r" (strex_result) : "r" (ptr), "r" (value) : "memory");
  return strex_result;
}

/**
 * @brief
 *   LL_SC_cmpxchg checks if *ptr is old_val and if it is old_val it guarantees that it gets changed to new_val
 *
 *   previous_value = *ptr;
 *   if previous_value == old_val, set *ptr = new_val;
 *   return previous_val;
 *
 * note: this pretends that whenever we see old_val we set it to new_val with 100% success,...
 * to do this, the function retries if the store conditional fails.

 * @param[in] ptr
 * @param[in] old_val
 * @param[in] new_val
 *
 * @return original value of *ptr (e.g. if it returns 'old_val', then the swap worked)
 */
static inline uint32_t atomic_cmpxchg( volatile uint32_t *ptr, uint32_t old_val, uint32_t new_val )
{
    while (1)
    {
        uint32_t val = atomic_LL(ptr);
        if (MDX2_UNLIKELY(val != old_val))
        {
            return val;
        }
        if (MDX2_LIKELY(atomic_SC(ptr,new_val) == 0))
        {
            return val;
        }
        // else try again until val is not old_val
    }
}


/**
 * @brief
 *   LL_SC_atomic_dec atomically decrements *ptr
 *
 *   previous_value = *ptr;
 *   new_value = previous_value - 1;
 *   *ptr = new_value;
 *   return previous_val;
 *
 * note: this pretends to work with 100% success,...
 * to do this, the function retries if the store conditional fails.
 *
 * e.g. if someone else modifies *ptr while this function is running,... 
 * then it starts over and tries again until it decreases *ptr by exactly 1.
 *
 * @param[in] ptr
 *
 * @return original value of *ptr prior to the successful atomic decrement.  e.g. if *ptr is 1, and we change it to 0, the function returns '1'.
 */
static inline uint32_t atomic_dec( volatile uint32_t *ptr )
{
    while (1)
    {
        uint32_t val = atomic_LL(ptr);
        uint32_t new_val = val-1;
        if (MDX2_LIKELY(atomic_SC(ptr,new_val) == 0))
        {
            return val;
        }
        // else try again
    }
}

#endif // ATOMIC_INTRINSICS_H
