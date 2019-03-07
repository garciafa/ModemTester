/*
 * Copyright 2019 Fabien Garcia
 * This file is part of ModemTester
 *
 * ModemTester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ModemTester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ModemTester.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
