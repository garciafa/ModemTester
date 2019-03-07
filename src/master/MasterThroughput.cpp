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
// Created by garciafa on 04/03/19.
//

#include "MasterThroughput.h"
#include <iostream>
#include "MasterFSM.h"

using std::placeholders::_1;
using std::placeholders::_2;

constexpr size_t nbCodes=(EndStandardCodes-BeginStandardCodes);

void MasterThroughput::operator() ()
{
    // Cancel all other operations on this stream
    //std::cout << std::this_thread::get_id() << " # Ask to cancel"<<std::endl;
    _ioStream->cancel();

    // Start reading asynchronously on stream
    //std::cout << std::this_thread::get_id() << " # Prepare handle receive"<<std::endl;
    _ioStream->async_read(boost::asio::buffer(_recvBuf,1),std::bind(&MasterThroughput::handleReceive,this,_1,_2));

    initReporting();

    //_askResultsTimer = new boost::asio::basic_waitable_timer<TheClock>(_ioStream->get_io_service(),std::chrono::seconds(2));
    //_askResultsTimer->async_wait(std::bind(&MasterThroughput::handleResultTimer,this,_1));
    std::array<uint8_t, nbCodes> sendBuf;
    for (uint8_t byte=BeginStandardCodes;byte<EndStandardCodes;++byte)
    {
        sendBuf[byte-BeginStandardCodes]=byte;
    }

    // Send as fast as possible
    while (!_stopRequested)
    {
        _ioStream->write_some(boost::asio::buffer(sendBuf,nbCodes));
    }

    endReporting();
    makeReport();
}

MasterThroughput::MasterThroughput (AISOBase *ioStream,TheClock::duration pingStatusReportPeriod,ThroughputReportFunctionType reportFunction)
: ReportingActivity(ioStream,pingStatusReportPeriod,reportFunction),_gotResults(false) ,_resultsReady(false) {}

void MasterThroughput::getResults()
{
    _resultsReady=false;
    std::array<uint8_t,1> data={EndStandardCodes};

    //std::cout << std::this_thread::get_id() << " # Ask to cancel"<<std::endl;
    _ioStream->cancel();

    //std::cout << std::this_thread::get_id() << " # Prepare receive results"<<std::endl;
    _ioStream->async_read(boost::asio::buffer(_recvBuf,_recvBuf.size()),std::bind(&MasterThroughput::receiveResults,this,_1,_2));

    _ioStream->write(boost::asio::buffer(data));

    // Wait for data to come back
    while (!_resultsReady)
    {
        usleep(5000);
    }
    //std::cout << std::this_thread::get_id() << " # Prepare handle receive"<<std::endl;
    _ioStream->async_read(boost::asio::buffer(_recvBuf,1),std::bind(&MasterThroughput::handleReceive,this,_1,_2));
}

void MasterThroughput::receiveResults (const boost::system::error_code &ec, std::size_t bytesAvailable)
{
    if (!ec)
    {
        //std::cout << std::this_thread::get_id() << std::endl;
        //std::cout << TimeLogger::now().count() << " # Received : " << bytesAvailable << " Bytes" << std::endl;

        if (bytesAvailable != 2 * sizeof(uint32_t))
            return;

        uint32_t totalReceived = 0;
        uint32_t totalUsec = 0;
        // TODO Make a simple protocol to test all data was received by master and send the throughput profile rather than the total

        uint32_t data;
        auto *ptr = (uint8_t *) &data;
        for (size_t i = 0; i < sizeof(uint32_t); ++i)
        {
            *ptr++ = _recvBuf[i];
        }
        totalReceived = ntohl(data);
        ptr = (uint8_t *) &data;
        for (size_t i = sizeof(uint32_t); i < 2 * sizeof(uint32_t); ++i)
        {
            *ptr++ = _recvBuf[i];
        }
        totalUsec = ntohl(data);

        /*
        double sec = totalUsec / 1000000.0;
        std::cout << "----------------------------------------------" << std::endl;
        std::cout << TimeLogger::now().count() << " # ThroughputMode report" << std::endl;
        std::cout << "\tReceived " << totalReceived << " Bytes in " << sec << " s" << std::endl;
        std::cout << "\tThroughput " << totalReceived / sec << " Bytes/s" << std::endl;
        std::cout << "\tThroughput " << 8 * totalReceived / sec << " b/s" << std::endl;
        std::cout << "----------------------------------------------" << std::endl;
        */
        _totalUsec=totalUsec;
        _totalReceived=totalReceived;
        _gotResults=_resultsReady = true;
    }
    else
    {
        if (ec.value()!=125)
        {
            std::cerr << "Error while reading results in MasterPinger mode : << " << ec.message() << "(" << ec.value() << ")" << std::endl;
            _stopRequested = true;
            MasterFSM::dispatch(EndTest()); // Stop if error on communication stream
        }
    }
}

void MasterThroughput::handleReceive (boost::system::error_code const &ec, std::size_t bytesAvailable)
{
    if (!ec)
    {
        for (size_t i=0;i<bytesAvailable;++i)
        {
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
        //std::cout << std::this_thread::get_id() << " # Prepare handle receive"<<std::endl;
        _ioStream->async_read(boost::asio::buffer(_recvBuf,1),std::bind(&MasterThroughput::handleReceive,this,_1,_2));
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

void MasterThroughput::makeReport ()
{
    getResults();
    _reportingFunction(_gotResults,_totalReceived,_totalUsec);
}

void MasterThroughput::noReportingFunction (bool b, uint32_t, uint32_t){}

TheClock::duration MasterThroughputFactory::_pingStatusPrintPeriod = std::chrono::seconds(0);
ThroughputReportFunctionType MasterThroughputFactory::_reportingFunction=&MasterThroughput::noReportingFunction;

MasterThroughputFactory::MasterThroughputFactory (AISOBase *ioStream) : CommunicatingRunnableFactory(ioStream) {}

const TheClock::duration &MasterThroughputFactory::getPingStatusPrintPeriod ()
{
    return _pingStatusPrintPeriod;
}

void MasterThroughputFactory::setPingStatusPrintPeriod (const TheClock::duration &pingStatusPrintPeriod)
{
    _pingStatusPrintPeriod = pingStatusPrintPeriod;
}

void MasterThroughputFactory::setReportingFunction (const ThroughputReportFunctionType &reportingFunction)
{
    _reportingFunction = reportingFunction;
}

MasterThroughput *MasterThroughputFactory::operator()()
{
    return new MasterThroughput(_ioStream,_pingStatusPrintPeriod,_reportingFunction);

}
