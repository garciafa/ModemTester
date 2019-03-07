//
// Created by garciafa on 05/03/19.
//

#ifndef MODEMTESTER_REMOTEPINGER_H
#define MODEMTESTER_REMOTEPINGER_H

#include "../Runnable.h"
#include "../CommunicatingRunnableFactory.h"

class RemotePinger : public Runnable {
public:
    void operator() () override;

    ~RemotePinger() override = default;

    void handleReceive (boost::system::error_code const &ec, std::size_t bytesAvailable);

    RemotePinger (AISOBase *ioStream);
protected:
    AISOBase *_ioStream;
    std::array<uint8_t, 2> _recvBuf;
};

using RemotePingerFactory = CommunicatingRunnableFactory<RemotePinger>;

#endif //MODEMTESTER_REMOTEPINGER_H
