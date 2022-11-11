#include <stdio.h>
#include <math.h>

#include "include/systime.h"
#include "include/fedoraengine.h"

float FE_DT = 0; // Time taken for each loop
const float FE_DT_TARGET = 1.0f / 60;
float FE_DT_MULTIPLIER = 1.0f; // The value that we need to multiply by to reach target FPS logic
uint32_t FE_FPS = 60;

static uint64_t current_time = 0;
static uint64_t last_time = 0;

void FE_ResetDT()
{
    current_time = 0;
    last_time = 0;
    FE_DT = 0;
    FE_FPS = 0;
}

int FE_CalculateDT()
{
	last_time = current_time;
	current_time = FE_QueryPerformanceCounter();
	FE_DT = (float)((current_time - last_time) / (float)FE_QueryPerformanceFrequency());

	// First frame will always produce invalid delta time, so disregard it
	FE_DT = (last_time == 0 ? 0 : FE_DT);

    // check for freezes or invalid dt
    if (FE_DT > 0.30f)
        FE_DT = 0.30f;

    FE_FPS = (1 / FE_DT);
    FE_DT_MULTIPLIER = (FE_DT / FE_DT_TARGET);

    return 1;
}