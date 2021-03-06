/*
 * Copyright 2019 Fabien Garcia
 * This file is part of ModemTester
 *
 * ModemTester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ModemTester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ModemTester.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

//
// Created by garciafa on 01/03/19.
//

#include <iostream>
#include <unistd.h>
#include "MasterPinger.h"
#include "MasterFSM.h"
#include "../Events.h"

using std::placeholders::_1;
using std::placeholders::_2;

AvailabilityReportFunctionType MasterPingerFactory::_reportingFunction=&MasterPinger::noReportingFunction;

#define AVAIL_THRESHOLD 3

void MasterPinger::operator() ()
{
    // Cancel all other operations on this stream
    _ioStream->cancel();

    usleep(50000);

    initReporting();

    // Start reading asynchronously on stream
    _ioStream->async_read_some(boost::asio::buffer(_recvBuf,1),std::bind(&MasterPinger::handleReceive,this,_1,_2));

    // Send a byte every 1/2 second
    boost::asio::basic_waitable_timer<TheClock> timer(_ioStream->get_io_service());
    std::array<uint8_t, 1> sendBuf;
    uint8_t &byte = sendBuf[0];
    byte=BeginStandardCodes;

    const uint8_t nbCodes = EndStandardCodes-BeginStandardCodes;
    while (!_stopRequested)
    {
        _ioStream->write(boost::asio::buffer(sendBuf,1));
        BOOST_LOG_TRIVIAL(trace) << "Sent " << (int)byte << std::endl;
        _sent++;
        _sendTimes[byte]=TheClock::now();
        byte= (uint8_t)((byte+1-BeginStandardCodes)%nbCodes+BeginStandardCodes);
        timer.expires_from_now(std::chrono::milliseconds(100)); // 10 Hz
        timer.wait();
        // consider the remote reachable if less than THRESHOLD packets are in flight
        if (_sent - (_received+_lost) > AVAIL_THRESHOLD)
        {
            _reachable=false;
        }
        else
        {
            _reachable=true;
        }

    }
    usleep(1000000*getAverageRtt()); // Allow for last packet to arrive

    endReporting();
    makeReport();

    //printStats();
}

MasterPinger::MasterPinger (AISOBase *ioStream,TheClock::duration pingStatusReportPeriod,AvailabilityReportFunctionType reportFunction)
: ReportingActivity<AvailabilityReportFunctionType>(ioStream,pingStatusReportPeriod,reportFunction), _sent(0), _lost(0), _received(0), _rtt(0.0), _reachable(false), _pingStatusReportPeriod(pingStatusReportPeriod), _reportingFunction(reportFunction)
{}

void MasterPinger::handleReceive (boost::system::error_code const &ec, std::size_t bytesAvailable)
{
    auto recvTime = TheClock::now();

    if (!ec)
    {
        for (size_t i=0;i<bytesAvailable;++i)
        {
            const uint8_t &byte = _recvBuf[i];

            if (byte >= BeginStandardCodes && byte <= EndStandardCodes)
            {
                BOOST_LOG_TRIVIAL(trace) << "Received " << (int)byte << std::endl;
                auto it = _sendTimes.find(byte);
                if (it!=_sendTimes.end())
                {
                    auto pRtt = recvTime-it->second;
                    _rtt+=std::chrono::duration_cast<std::chrono::duration<double>>(pRtt).count();
                    ++_received;
                    // We consider everything sent before 'byte' was lost
                    _lost += std::distance(_sendTimes.begin(),it);
                    _sendTimes.erase(_sendTimes.begin(),it);
                    _sendTimes.erase(it);
                }
                else
                {
                    std::cerr << " Byte " << (int) byte << " not in _sendTimes map" << std::endl;
                }
            }
            else
            {
                //std::cout << TimeLogger::now().count() << " # Got " << (EventId_t)_recvBuf[i] << std::endl;
                switch (_recvBuf[i])
                {
                    case RemoteAck_id: // If remote ack arrives late
                        MasterFSM::dispatch(RemoteAck());
                        return;
                    case EndTest_id:
                        MasterFSM::dispatch(EndTest());
                        return;
                    case EndModeCommand_id:
                        MasterFSM::dispatch(EndModeCommand());
                        return;
                    default:
                        break; // Al other commands ignored
                }
            }
        }
        _ioStream->async_read_some(boost::asio::buffer(_recvBuf,1),std::bind(&MasterPinger::handleReceive,this,_1,_2));
    }
    else
    {
        if (ec.value()!=125)
        {
            std::cerr << "Error during read in MasterPinger mode : << " << ec.message() << "(" << ec.value() << ")" << std::endl;
            _stopRequested = true;
            MasterFSM::dispatch(EndTest()); // Stop if error on communication stream
        }
    }
}

unsigned long MasterPinger::getLost () const
{
    return _lost;
}

unsigned long MasterPinger::getReceived () const
{
    return _received;
}

double MasterPinger::getAverageRtt () const
{
    return _rtt/_received;
}

void MasterPinger::printStats () const
{
    BOOST_LOG_TRIVIAL(info) << "\n----------------------------------------------" << std::endl
    << "AvailabilityMode report" << std::endl
    << "\tSent " << _sent << " bytes\n"
    << "\tReceived " << _received << " bytes\n"
    << "\tLost " << _lost << " bytes\n"
    << "\tAvRTT " << getAverageRtt()*1000 << " ms\n"
    << "----------------------------------------------" << std::endl;
}

unsigned long MasterPinger::getSent () const
{
    return _sent;
}

void MasterPinger::noReportingFunction (bool reachable,unsigned long sent,unsigned long lost,unsigned long received,double avRtt){}

void MasterPinger::makeReport ()
{
    _reportingFunction(_reachable,_sent,_lost,_received,getAverageRtt());
}

TheClock::duration MasterPingerFactory::_pingStatusPrintPeriod = std::chrono::seconds(0);

const TheClock::duration &MasterPingerFactory::getPingStatusPrintPeriod ()
{
    return _pingStatusPrintPeriod;
}

void MasterPingerFactory::setPingStatusPrintPeriod (const TheClock::duration &pingStatusPrintPeriod)
{
    _pingStatusPrintPeriod = pingStatusPrintPeriod;
}

MasterPinger* MasterPingerFactory::operator()()
{
    return new MasterPinger(_ioStream,_pingStatusPrintPeriod,_reportingFunction);
}

MasterPingerFactory::MasterPingerFactory (AISOBase *ioStream)
    : CommunicatingRunnableFactory(ioStream)
    {}

void MasterPingerFactory::setReportingFunction (const AvailabilityReportFunctionType &reportingFunction)
{
    _reportingFunction = reportingFunction;
}
