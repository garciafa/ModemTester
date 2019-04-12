//
// Created by garciafa on 26/03/19.
//

#ifndef MODEMTESTER_MASTERCONTROLLER_H
#define MODEMTESTER_MASTERCONTROLLER_H
#include "../AISO.h"
#include "MasterFSM.h"

class MasterController {
public:
    explicit MasterController (AISOBase *ioSstream);

    void start();

    void stop();

    void periodicAction(boost::system::error_code const & ec);

    void setRemoteReachability(bool reachable);

private:
    AISOBase *_ioSstream;
    boost::asio::basic_waitable_timer<TheClock> _timer;
    bool _remoteReachable;
    bool _tryingAvalability;
};

#endif //MODEMTESTER_MASTERCONTROLLER_H
