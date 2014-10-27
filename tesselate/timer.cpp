/* file: timer.cpp
   author: (c) James Gain, 2006
   project: ScapeSketch - sketch-based design of procedural landscapes
   notes: fairly accurate timing routines
            reverted to getimeofday for unix compatibility
   changes:
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "timer.h"

// start: shell to call timeofday with start
void Timer::start()
{
    gettimeofday(&tbegin, &zone);
}

// stop: shell to call timeofday with stop
void Timer::stop()
{
    gettimeofday(&tend, &zone);
}

// peek:        get the current elapsed time between begin and end
// return:      begin-end
float Timer::peek()
{
    float total_time;
    long time_in_sec, time_in_ms;

    time_in_sec = tend.tv_sec - tbegin.tv_sec;
    time_in_ms = tend.tv_usec - tbegin.tv_usec;
    total_time = ((float) time_in_ms)/1000000.0;
    total_time += ((float) time_in_sec);
    return total_time;
}
