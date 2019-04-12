//
// Created by garciafa on 26/03/19.
//

#ifndef MODEMTESTER_DISPATCHINGRUNNABLE_H
#define MODEMTESTER_DISPATCHINGRUNNABLE_H

#include "Runnable.h"

template <typename FSM, typename E>
class DispatchingRunnable : public Runnable {
public:
    void operator()() override
    {
        E e;
        FSM::dispatch(E());
    }
};

#endif //MODEMTESTER_DISPATCHINGRUNNABLE_H
