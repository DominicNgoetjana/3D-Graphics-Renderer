#ifndef TimerC
#define TimerC
/* file: timer.h
   author: (c) James Gain, 2006
   project: ScapeSketch - sketch-based design of procedural landscapes
   notes: fairly accurate timing routines
   changes:
*/

//#include <Carbon/Carbon.h>

#include <sys/time.h>

class Timer
{

private:
    struct timeval tbegin, tend;
    struct timezone zone;

public:

    // start: shell to call timeofday with start
    void start();

    // stop: shell to call timeofday with stop
    void stop();

    // peek:    get the current elapsed time between begin and end
    // return:      begin-end
    float peek();
};

#endif