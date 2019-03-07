//
// Created by garciafa on 06/03/19.
//

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