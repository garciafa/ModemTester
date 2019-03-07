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

#ifndef MODEMTESTER_RUNNABLE_H
#define MODEMTESTER_RUNNABLE_H

#include <thread>

class Runnable {
public:
    Runnable ();

    virtual ~Runnable () = default;

    virtual void operator()() =0;

    virtual void lifecycle() final;
    virtual void begin();
    virtual void end();

    bool isRunning();

    void start();

    void stop();

protected:
    volatile bool _running;
    std::thread *_thr;
    volatile bool _stopRequested;
};

#endif //MODEMTESTER_RUNNABLE_H
