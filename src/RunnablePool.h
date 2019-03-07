//
// Created by garciafa on 06/03/19.
//

#ifndef MODEMTESTER_RUNNABLEPOOL_H
#define MODEMTESTER_RUNNABLEPOOL_H

#include <vector>
#include <functional>
#include <mutex>
#include "Runnable.h"

class RunnablePool : public Runnable {
public:
    RunnablePool ();
    ~RunnablePool () override;
    void operator() () override final;
    void addAndStartRunnable (Runnable *runnable);

protected:
    std::vector<Runnable*> _runnables;
    std::mutex _mut;
};

#endif //MODEMTESTER_RUNNABLEPOOL_H
