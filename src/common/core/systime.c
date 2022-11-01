#include <stdint.h>
#include <stdbool.h>


// include the timing libraries for windows and linux
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

/* Provides an implementation for using PerformanceCounter and PerformanceFrequency to get the time in seconds.
Implements on windows and linux. */

static bool InitTime = false;

#ifdef _WIN32
static LARGE_NTEGER StartCounter;
static LARGE_NTEGER CounterFrequency;
#else
static struct timeval StartTime;
static struct timespec StartSpec;
#endif

void FE_Time_Init()
{
    if (InitTime) {
        return;
    }
    InitTime = true;

    #ifdef _WIN32
    bool res = QueryPerformanceFrequency(&CounterFrequency);
    if (!res) {
        error("Failed to get performance frequency (Must be WINXP or higher)");
    }
    #else
    clock_gettime(CLOCK_MONOTONIC, &StartSpec);
    gettimeofday(&StartTime, NULL);
    #endif
}


uint64_t FE_QueryPerformanceCounter()
{
    #ifdef _WIN32
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return counter.QuadPart;
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
    #endif
}

uint64_t FE_QueryPerformanceFrequency()
{
    #ifdef _WIN32
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return frequency.QuadPart;
    #else
        return 1000000000;
    #endif
}

uint64_t FE_GetTicks64()
{
    #ifdef _WIN32
        LARGE_INTEGER now;
        bool res = QueryPerformanceCounter(&now);
        if (!res) {
            error("Failed to get performance counter (Must be WINXP or higher)");
        }
        return (uint64_t)(((now.QuadPart - StartTime.QuadPart) * 1000) / CounterFrequency.QuadPart);
    #else
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        return (uint64_t)(((uint64_t)(now.tv_sec - StartSpec.tv_sec) * 1000) + ((now.tv_nsec - StartSpec.tv_nsec) / 1000000));
    #endif
}