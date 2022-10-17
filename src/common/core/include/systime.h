#ifndef _H_SYSTIME
#define _H_SYSTIME

#include <stdint.h>


/** Returns the high resolution time of the counter in milliseconds
 * \return The time in milliseconds
 */
uint64_t FE_QueryPerformanceCounter();


/** Returns the frequency of the counter in milliseconds
 * \return The frequency in milliseconds
 */
uint64_t FE_QueryPerformanceFrequency();


/** Returns the time since the program started in milliseconds
 * \return The time in milliseconds
 */
uint64_t FE_GetTicks64();


/** Initialises the time library
 */
void FE_InitTime();


#endif