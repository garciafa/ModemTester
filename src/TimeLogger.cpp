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
