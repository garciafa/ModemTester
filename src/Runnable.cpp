//
// Created by garciafa on 01/03/19.
//

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
