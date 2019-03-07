//
// Created by garciafa on 04/03/19.
//

#include "TimeLogger.h"

TheClock::time_point TimeLogger::_begin = TheClock::now();

void TimeLogger::reset()
{
    _begin = TheClock::now();
}

std::chrono::duration<double> TimeLogger::now ()
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(TheClock::now()-_begin);
}

std::chrono::duration<double> TimeLogger::toDurationDouble (TheClock::time_point &t)
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(t-_begin);
}
