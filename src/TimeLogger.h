//
// Created by garciafa on 04/03/19.
//

#ifndef MODEMTESTER_TIMELOGGER_H
#define MODEMTESTER_TIMELOGGER_H

#include <chrono>

using TheClock = std::chrono::high_resolution_clock;

class TimeLogger {
public:
    static void reset ();

    static std::chrono::duration<double> now();

    static std::chrono::duration<double> toDurationDouble(TheClock::time_point &t);
private:
    static TheClock::time_point _begin;
};

#endif //MODEMTESTER_TIMELOGGER_H
