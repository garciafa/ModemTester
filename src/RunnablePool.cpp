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

#include "RunnablePool.h"
#include <unistd.h>
#include <iostream>

void RunnablePool::addAndStartRunnable (Runnable *runnable)
{
    std::lock_guard<std::mutex> lock(_mut);
    _runnables.push_back(runnable);
    runnable->start();
}

RunnablePool::RunnablePool () : Runnable()
{
    start();
}

RunnablePool::~RunnablePool ()
{
    for (auto runnable : _runnables)
    {
        delete runnable;
    }
}

bool isnull(Runnable *run)
{
    return run==nullptr;
}

void RunnablePool::operator() ()
{
    // Garbage collect not running Runnables every second
    while(!_stopRequested)
    {
        {
            std::lock_guard<std::mutex> lock(_mut);
            for (auto it = _runnables.begin(); it != _runnables.end(); it++)
            {
                (*it)->stop();
                delete (*it);
                *it=nullptr;
            }
            auto it = std::remove_if(_runnables.begin(),_runnables.end(),isnull);
            _runnables.erase(it,_runnables.end());
        }
        sleep(1);
    }
}