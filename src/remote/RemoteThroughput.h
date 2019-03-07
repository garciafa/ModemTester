//
// Created by garciafa on 05/03/19.
//

#ifndef MODEMTESTER_REMOTETHROUGHPUT_H
#define MODEMTESTER_REMOTETHROUGHPUT_H

#include "../Runnable.h"
#include "../CommunicatingRunnableFactory.h"
#include "../TimeLogger.h"

class RemoteThroughput : public Runnable {
public:
    void operator() () override;

    ~RemoteThroughput() override = default;

    void handleReceive (boost::system::error_code const &ec, std::size_t bytesAvailable);

    explicit RemoteThroughput (AISOBase *ioStream);

    void sendResults();

protected:
    AISOBase *_ioStream;
    std::array<uint8_t, 255> _recvBuf; // FIXME the size should be settable
    std::map <TheClock::time_point,size_t> _recvedBytes;
};

using RemoteThroughputFactory = CommunicatingRunnableFactory<RemoteThroughput>;

#endif //MODEMTESTER_REMOTETHROUGHPUT_H
