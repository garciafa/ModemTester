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

std::function<void(unsigned long,unsigned long,unsigned long,double)> MasterPingerFactory::_reportingFunction=&MasterPinger::noReportingFunction;

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
        //std::cout << TimeLogger::now().count() << " # Sent " << (int)byte << std::endl;
        _sent++;
        _sendTimes[byte]=TheClock::now();
        byte= (uint8_t)((byte+1-BeginStandardCodes)%nbCodes+BeginStandardCodes);
        timer.expires_from_now(std::chrono::milliseconds(500));
        timer.wait();
    }
    usleep(1000000*getAverageRtt()); // Allow for last packet to arrive

    endReporting();
    makeReport();

    //printStats();
}

MasterPinger::MasterPinger (AISOBase *ioStream,TheClock::duration pingStatusReportPeriod,std::function<void(unsigned long,unsigned long,unsigned long,double)> reportFunction)
: ReportingActivity<AvailabilityReportFunctionType>(ioStream,pingStatusReportPeriod,reportFunction), _sent(0), _lost(0), _received(0), _rtt(0.0), _pingStatusReportPeriod(pingStatusReportPeriod), _reportingFunction(reportFunction)
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
                //std::cout << TimeLogger::now().count() << " # Received " << (int)byte << std::endl;
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
    std::cout << "----------------------------------------------" << std::endl;
    std::cout << TimeLogger::now().count() << " # AvailabilityMode report" << std::endl;
    std::cout << "\tSent " << _sent << " bytes\n";
    std::cout << "\tReceived " << _received << " bytes\n";
    std::cout << "\tLost " << _lost << " bytes\n";
    std::cout << "\tAvRTT " << getAverageRtt()*1000 << " ms\n";
    std::cout << "----------------------------------------------" << std::endl;
}

unsigned long MasterPinger::getSent () const
{
    return _sent;
}

void MasterPinger::noReportingFunction (unsigned long sent,unsigned long lost,unsigned long received,double avRtt){}

void MasterPinger::makeReport ()
{
    _reportingFunction(_sent,_lost,_received,getAverageRtt());
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

void MasterPingerFactory::setReportingFunction (const std::function<void (unsigned long, unsigned long, unsigned long, double)> &reportingFunction)
{
    _reportingFunction = reportingFunction;
}
