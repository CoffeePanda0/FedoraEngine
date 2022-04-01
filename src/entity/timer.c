#include "../include/game.h"

static FE_Timer **timers; // basic list of timers
static size_t list_size;
static size_t timer_count;

unsigned long int FE_LastUpdate = 0;

static int AddToList(FE_Timer *t)
{
    if (t == NULL) {
        warn("Attempted to add a NULL timer to the list");
        return -1;
    }

    if (timer_count == 0) {
        timers = xmalloc(sizeof(FE_Timer *));
        timers[0] = t;
        timer_count = 1;
        list_size = 1;
        return 1;
    }

    // find the next free space to use if space is available in the middle of the list
    for (size_t i = 0; i < list_size; i++) {
        if (timers[i] == NULL) {
            timers[i] = t;
            timer_count++;
            return 1;
        }
    }

    // if we're here, we need to add to the end of the list
    timers = xrealloc(timers, sizeof(FE_Timer *) * (timer_count + 1));
    timers[++list_size] = t;
    timer_count++;
    return 1;
}


int FE_RemoveTimer(FE_Timer *t)
{
    if (t == NULL) {
        warn("Attempted to remove a null timer");
        return -1;
    }

    if (list_size == 1) { // if the item is the only item in the list and at the start of the list
        timers[0] = NULL;
        free(t);
        free(timers);
        timer_count--;
        list_size--;
        return 1;
    }

    size_t index; // index in the list of the item to remove
    bool found = false;
    for (size_t i = 0; i < list_size; i++) {
        if (timers[i] == t) {
            index = i;
            found = true;
            break;
        }
    }

    if (!found) {
        warn("Attempted to remove a timer that wasn't in the list");
        return -1;
    }

    // if the item is at the end of the list
    if (index == list_size - 1) {
        timers = xrealloc(timers, sizeof(FE_Timer *) * (--list_size)); // shrink the list
    }

    timers[index] = NULL;
    timer_count--;

    free(t);
    return 1;

}

void FE_StartTimer(FE_Timer *timer)
{
    if (timer == NULL) {
        warn("Attempted to start a null timer");
        return;
    }

    timer->paused = false;
}

void FE_PauseTimer(FE_Timer *timer)
{
    if (timer == NULL) {
        warn("Attempted to stop a null timer");
        return;
    }

    timer->paused = true;
}

void FE_RestartTimer(FE_Timer *timer)
{
    if (timer == NULL) {
        warn("Attempted to restart a null timer");
        return;
    }

    timer->paused = false;
    timer->elapsed = 0;
}

FE_Timer *FE_CreateTimer(unsigned long duration, bool started, bool repeats, void (*callback), void *params)
{
    if (!callback) {
        warn("Callback is null! (FE_CreateTimer)");
        return NULL;
    }

    FE_Timer *t = xmalloc(sizeof(FE_Timer));
    t->duration = duration;
    t->elapsed = 0;
    t->paused = !started;
    t->repeats = repeats;
    t->callback = callback;
    t->params = params;

    AddToList(t);

    return t;
}

void FE_UpdateTimers() // Updates all timers, checked if time elapsed
{
    unsigned long time_passed = SDL_GetTicks() - FE_LastUpdate; // calculate time passed since last update

    for (size_t i = 0; i < list_size; i++) {
        if (timers[i] == NULL) { // if the timer is null, skip it
            continue;
        }

        if (timers[i]->paused) { // if the timer is paused, skip it
            continue;
        }

        timers[i]->elapsed += time_passed; // add time passed since last update to the timer

        if (timers[i]->elapsed >= timers[i]->duration) { // if the timer has reached its duration
            timers[i]->callback(timers[i]->params); // call the callback function
            if (timers[i]->repeats) { // if the timer repeats, reset the timer
                timers[i]->elapsed = 0;
            } else {
                FE_RemoveTimer(timers[i]); // otherwise, remove the timer
            }
        }
    }
    FE_LastUpdate = SDL_GetTicks();
}

int FE_CleanTimers() // Removes all timers, frees memory
{
    if (timers == NULL)
        return 0;
    
    for (size_t i = 0; i < list_size; i++) {
        if (timers[i] == NULL) {
            continue;
        }

        free(timers[i]);
    }

    free(timers);
    timers = NULL;
    timer_count = 0;
    list_size = 0;
    return 1;
}