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

#include <functional>
#include <iostream>
#include <unistd.h>
#include "Runnable.h"
#include "TimeLogger.h"

Runnable::Runnable () : _running(false), _thr(nullptr), _stopRequested(false) {}

bool Runnable::isRunning ()
{
    return _running;
}

void Runnable::start ()
{
    _thr = new std::thread(std::bind(&Runnable::lifecycle,this));
}

void Runnable::stop ()
{
    _stopRequested = true;
    if (_thr)
    {
        _thr->join();
        delete (_thr);
        _thr = nullptr;
    }
}


void Runnable::lifecycle ()
{
    begin();
    _running=true;
    (*this)();
    _running=false;
    end();
}

void Runnable::begin ()
{
}

void Runnable::end()
{
}
