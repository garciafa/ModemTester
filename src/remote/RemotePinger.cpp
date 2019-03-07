//
// Created by garciafa on 05/03/19.
//

#include <iostream>
#include "RemotePinger.h"
#include "../TimeLogger.h"
#include "../Events.h"
#include "RemoteFSM.h"

using std::placeholders::_1;
using std::placeholders::_2;

void RemotePinger::operator() ()
{
    // Cancel all other operations on this stream
    _ioStream->cancel();

    // Start reading asynchronously on stream
    _ioStream->async_read_some(boost::asio::buffer(_recvBuf,1),std::bind(&RemotePinger::handleReceive,this,_1,_2));
}

void RemotePinger::handleReceive (boost::system::error_code const &ec, std::size_t bytesAvailable)
{
    if (!ec)
    {
        for (size_t i=0;i<bytesAvailable;++i)
        {
            const uint8_t &byte = _recvBuf[i];

            if (byte>=BeginStandardCodes && byte<=EndStandardCodes)
            {
                // If it is not a command we send it back
                _ioStream->write(boost::asio::buffer(_recvBuf,1));
                //std::cout << TimeLogger::now().count() << " # Sent " << (int)_recvBuf[0] << std::endl;
            }
            else
            {
                //std::cout << TimeLogger::now().count() << " # Got " << (EventId_t)_recvBuf[0] << std::endl;
                switch ((EventId_t) byte)
                {
                    case EndTest_id:
                        RemoteFSM::dispatch(EndTest());
                        return;
                        break;
                    case EndModeCommand_id:
                        RemoteFSM::dispatch(EndModeCommand());
                        return;
                        break;
                    default:
                        break; // Ignore other events
                }
            }
        }
        _ioStream->async_read_some(boost::asio::buffer(_recvBuf,1),std::bind(&RemotePinger::handleReceive,this,_1,_2));
    }
    else
    {
        if (ec.value()!=125)
        {
            std::cerr << "Error during read in RemotePinger mode : << " << ec.message() << "(" << ec.value() << ")" << std::endl;
            _stopRequested = true;
            RemoteFSM::dispatch(EndTest()); // Stop if error on communication stream
        }
    }
}

RemotePinger::RemotePinger (AISOBase *ioStream) : _ioStream(ioStream) {}
