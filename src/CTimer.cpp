#include "CTimer.h"
#include "common.h"

CTimer::CTimer()
{
    _start = clock();
}

CTimer::~CTimer()
{
    _end = clock();
    cout<<float(_end - _start)/1000000<<endl;
}

