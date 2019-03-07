//
// Created by garciafa on 01/03/19.
//

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
