#ifndef CTIMER_H
#define CTIMER_H
#include <time.h>

class CTimer
{
    public:
        CTimer();
        ~CTimer();

    private:
        clock_t _start;
        clock_t _end;
};

#endif