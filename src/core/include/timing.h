#ifndef H_TIMING
#define H_TIMING

extern float FE_DT; // Time taken for one update to complete
extern const float FE_DT_TARGET; // Our target DT
extern float FE_DT_MULTIPLIER; // Multiplier to be applied to calculations to execute at same rate as target DT
extern uint32_t FE_FPS; // Game FPS


/** Calculates all variables relating to timing (DT value, Multiplier and game FPS)
 * \return 1 on success
 */
int FE_CalculateDT();


/** Times how long a function takes to complete
 * \param callback The function pointer to time
 * \param params The parameters to pass to the function
 * \returns How long the function took to complete
*/
float FE_TimeFunction(void (*callback)(), void *params);

void FE_ResetDT();

#endif