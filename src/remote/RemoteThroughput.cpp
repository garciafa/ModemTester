//
// Created by garciafa on 05/03/19.
//

#include "RemoteThroughput.h"
#include <iostream>
#include "../TimeLogger.h"
#include "../Events.h"
#include "RemoteFSM.h"

using std::placeholders::_1;
using std::placeholders::_2;

void RemoteThroughput::operator() ()
{
    // Cancel all other operations on this stream
    _ioStream->cancel();

    // Start reading asynchronously on stream
    _ioStream->async_read_some(boost::asio::buffer(_recvBuf,1),std::bind(&RemoteThroughput::handleReceive,this,_1,_2));
}

void RemoteThroughput::handleReceive (boost::system::error_code const &ec, std::size_t bytesAvailable)
{
    auto recvTime = TheClock::now();
    if (!ec)
    {
        // Look for commands in the data
        for (std::size_t i=0;i<bytesAvailable;++i)
        {
            if (_recvBuf[i] >= EndStandardCodes)
            {
                //std::cout << TimeLogger::now().count() << " # Got " << (EventId_t) _recvBuf[i] << std::endl;

                if (_recvBuf[i] == EndStandardCodes)
                {
                    // Send the result of computation to Master
                    sendResults();
                }
                else
                {
                    switch (_recvBuf[i])
                    {
                        case EndTest_id:
                            RemoteFSM::dispatch(EndTest());
                            return;
                        case EndModeCommand_id:
                            RemoteFSM::dispatch(EndModeCommand());
                            return;
                        default:
                            std::cout << TimeLogger::now().count() << " # Got " << (EventId_t) _recvBuf[i]
                                      << " in RemoteThroughputMode" << std::endl;
                    }
                }
            }
        }

        // Store data
        _recvedBytes[recvTime] = bytesAvailable;
        //std::cout << TimeLogger::now().count() << " # Got " << bytesAvailable << " Bytes" << std::endl;

        _ioStream->async_read_some(boost::asio::buffer(_recvBuf),std::bind(&RemoteThroughput::handleReceive,this,_1,_2));
    }
    else
    {
        if (ec.value()!=125)
        {
            std::cerr << "Error during read in RemoteThroughput mode : " << ec.message() << "(" << ec.value() << ")" << std::endl;
            _stopRequested = true;
            RemoteFSM::dispatch(EndTest()); // Stop if error on communication stream
        }
    }
}

RemoteThroughput::RemoteThroughput (AISOBase *ioStream) : _ioStream(ioStream) {}

void RemoteThroughput::sendResults ()
{
    std::cout << TimeLogger::now().count() << " # Sending results" << std::endl;

    uint32_t totalReceived=0;
    uint32_t totalUsec=0; // Enough for more than one hour
    // TODO Make a simple protocol to test all data was received by master and send the throughput profile rather than the total
    for (auto pair: _recvedBytes)
    {
        totalReceived+=pair.second;
    }
    totalReceived-= std::prev(_recvedBytes.end())->second; // Do not count the last one

    auto timeSpan = std::prev(_recvedBytes.end())->first-_recvedBytes.begin()->first;

    totalUsec = (uint32_t)(std::chrono::duration_cast<std::chrono::duration<double>>(timeSpan).count()*1000000.0);

    std::array<uint8_t, 2*sizeof(uint32_t)> sendBuf; // Buffer to hold two uint32

    uint32_t data = htonl(totalReceived);
    auto *ptr = (uint8_t*)&data;
    for (size_t i=0;i<sizeof(data);++i)
    {
        sendBuf[i]=*ptr++;
    }
    data = htonl(totalUsec);
    ptr = (uint8_t*)&data;
    for (size_t i=sizeof(data);i<2*sizeof(data);++i)
    {
        sendBuf[i]=*ptr++;
    }
    _ioStream->write(boost::asio::buffer(sendBuf));
    std::cout << "Sent " << sendBuf.size() << " Bytes" << std::endl;
    double sec = totalUsec/1000000.0;
    std::cout << "----------------------------------------------" << std::endl;
    std::cout << TimeLogger::now().count() << std::endl;
    std::cout << "\tReceived " << totalReceived << " Bytes in " << sec << " s" << std::endl;
    std::cout << "\tThroughput " << totalReceived/sec << " Bytes/s" << std::endl;
    std::cout << "\tThroughput " << 8*totalReceived/sec << " b/s" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
}
