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
#include "../RunnablePool.h"
#include "../EventsScript.h"

using std::placeholders::_1;
using std::placeholders::_2;

constexpr size_t nbCodes=(EndStandardCodes-BeginStandardCodes);

void MasterThroughput::operator() ()
{
    // Cancel all other operations on this stream
    _ioStream->cancel();

    // Start reading asynchronously on stream
    _ioStream->async_read(boost::asio::buffer(_recvBuf, 1), std::bind(&MasterThroughput::handleReceive, this, _1, _2));

    initReporting();

    std::array<uint8_t, nbCodes> sendBuf;
    for (uint8_t byte = BeginStandardCodes; byte < EndStandardCodes; ++byte)
    {
        sendBuf[byte - BeginStandardCodes] = byte;
    }

    size_t totalSent = 0;
    // Send an amount of data as fast as possible
    for (int it = 0; it < 10 && !_stopRequested; ++it)
    {
        _ioStream->write_some(boost::asio::buffer(sendBuf, nbCodes));
        totalSent += nbCodes;
    }
    BOOST_LOG_TRIVIAL(trace) << "Done sending, will wait " << totalSent / (_estimatedThroughput / 8.0) << " s\n";
    // Wait for data to arrive on the other side
    usleep(1000000 * totalSent / (_estimatedThroughput / 8.0)); // 255 Bytes sent at _estimatedThroughput b/s turned in us

    BOOST_LOG_TRIVIAL(trace) << "Finished wait\n";
    endReporting();
    makeReport();
    BOOST_LOG_TRIVIAL(trace) << "Report made\n";


    RunnablePool::getInstance()->addAndStartRunnable(new DispatchingRunnable<MasterFSM,EndModeCommand>());
}

MasterThroughput::MasterThroughput (AISOBase *ioStream,TheClock::duration pingStatusReportPeriod,ThroughputReportFunctionType reportFunction,uint32_t estimatedThroughput)
: ReportingActivity(ioStream,pingStatusReportPeriod,reportFunction),_gotResults(false) ,_resultsReady(false), _estimatedThroughput(estimatedThroughput) {}

void MasterThroughput::getResults()
{
    _resultsReady=false;
    std::array<uint8_t,1> data={EndStandardCodes};

    _ioStream->cancel();
    _ioStream->async_read(boost::asio::buffer(_recvBuf,_recvBuf.size()),std::bind(&MasterThroughput::receiveResults,this,_1,_2));
    _ioStream->write(boost::asio::buffer(data));

    // TODO needs a clean timeout !!!
    _receiveResultsTimer = new boost::asio::basic_waitable_timer<TheClock>(_ioStream->get_io_service(),std::chrono::seconds(3));
    _receiveResultsTimer->async_wait(std::bind(&MasterThroughput::timeoutReceiveResults,this,_1));

    // Wait for data to come back
    while (!_resultsReady)
    {
        usleep(5000);
    }

    _ioStream->async_read(boost::asio::buffer(_recvBuf,1),std::bind(&MasterThroughput::handleReceive,this,_1,_2));
}

void MasterThroughput::timeoutReceiveResults (const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cout << "Timeout waiting results => sending again\n";
        std::array<uint8_t,1> data={EndStandardCodes};
        _ioStream->write(boost::asio::buffer(data));
        _receiveResultsTimer->expires_from_now(std::chrono::seconds(3));
        _receiveResultsTimer->async_wait(std::bind(&MasterThroughput::timeoutReceiveResults,this,_1));
    }
}

void MasterThroughput::receiveResults (const boost::system::error_code &ec, std::size_t bytesAvailable)
{
    if (!ec)
    {
        delete _receiveResultsTimer;

        if (bytesAvailable != 2 * sizeof(uint32_t))
            return; // Not enought data => this will erase it, is it good ?

        uint32_t totalReceived = 0;
        uint32_t totalUsec = 0;

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

TheClock::duration MasterThroughputFactory::_throughputResultPrintPeriod = std::chrono::seconds(0);
ThroughputReportFunctionType MasterThroughputFactory::_reportingFunction=&MasterThroughput::noReportingFunction;
uint32_t MasterThroughputFactory::_estimatedThroughput = 9600;

MasterThroughputFactory::MasterThroughputFactory (AISOBase *ioStream) : CommunicatingRunnableFactory(ioStream) {}

const TheClock::duration &MasterThroughputFactory::getThroughputResultPrintPeriod ()
{
    return _throughputResultPrintPeriod;
}

void MasterThroughputFactory::setThroughputResultPrintPeriod (const TheClock::duration &throughputResultPrintPeriod)
{
    _throughputResultPrintPeriod = throughputResultPrintPeriod;
}

void MasterThroughputFactory::setReportingFunction (const ThroughputReportFunctionType &reportingFunction)
{
    _reportingFunction = reportingFunction;
}

MasterThroughput *MasterThroughputFactory::operator()()
{
    return new MasterThroughput(_ioStream,_throughputResultPrintPeriod,_reportingFunction,_estimatedThroughput);

}

const uint32_t MasterThroughputFactory::getEstimatedThroughput ()
{
    return _estimatedThroughput;
}

void MasterThroughputFactory::setEstimatedThroughput (const uint32_t &estimatedThroughput)
{
    _estimatedThroughput = estimatedThroughput;
}
