#ifndef H_TIMER
#define H_TIMER

#include "../../core/lib/bool.h"

typedef struct FE_Timer {
    unsigned long duration; // How long the timer will last in milliseconds
    unsigned long elapsed; // How long the timer has been running in milliseconds

    bool paused; // Whether the timer is paused or not
    bool repeats; // Whether the timer repeats or deletes itself after it's done

    void (*callback)(); // The function to call when the timer is done
    void *params; // The parameters to pass to the callback function
} FE_Timer;


/** Creates a new timer
    *\param duration How long the timer will last in milliseconds
    *\param started Whether the timer should start immediately once created
    *\param repeats Whether the timer repeats or deletes itself after it's done
    *\param callback The function to call when the timer is done
    *\param params The parameters to pass to the callback function
    *\return A pointer to the new timer
*/
FE_Timer *FE_CreateTimer(unsigned long duration, bool started, bool repeats, void (*callback)(), void *params);


/* Main timer loop, must be called for the timers to run */
void FE_UpdateTimers();


/** Removes a timer and frees resources used
*\param timer The timer to remove
*/
int FE_RemoveTimer(FE_Timer *timer);


/** Restarts a timer 
*\param timer The timer to restart
*/
void FE_RestartTimer(FE_Timer *timer);


/** Pauses a timer 
*\param timer The timer to pause
*/
void FE_PauseTimer(FE_Timer *timer);


/** Resumes or starts a paused timer 
*\param timer The timer to pause
*/
void FE_PauseTimer(FE_Timer *timer);


/* Destroys all timers */
int FE_CleanTimers();

#endif