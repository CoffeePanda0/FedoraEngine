#ifndef H_TIMER
#define H_TIMER

typedef struct FE_Timer {
    unsigned long duration; // How long the timer will last in milliseconds
    unsigned long elapsed; // How long the timer has been running in milliseconds

    bool paused; // Whether the timer is paused or not
    bool repeats; // Whether the timer repeats or deletes itself after it's done

    void (*callback)(); // The function to call when the timer is done
    void *params; // The parameters to pass to the callback function
} FE_Timer;

FE_Timer *FE_CreateTimer(unsigned long duration, bool started, bool repeats, void (*callback), void *params);
void FE_UpdateTimers();
int FE_RemoveTimer(FE_Timer *timer);
int FE_CleanTimers();

#endif