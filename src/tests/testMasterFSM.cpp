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
// Created by garciafa on 28/02/19.
//

#include <cstdlib>
#include <iostream>
#include <functional>
#include "../master/MasterFSM.h"
#include "../time_def.h"
#include "../EventsScript.h"
#include "../logging.h"
#include <Ivy/Ivycpp.h>

FSM_INITIAL_STATE(MasterFSM,Idle);

using boost::asio::io_service;

using boost::asio::ip::tcp;

boost::asio::basic_waitable_timer<TheClock> *printTimer;

constexpr auto printStatusPeriod = std::chrono::seconds(1);

void printState(boost::system::error_code const & ec)
{
    printTimer->expires_from_now(printStatusPeriod);
    BOOST_LOG_TRIVIAL(info) << "Current state : " << *MasterFSM::current_state_ptr << std::endl;
    printTimer->async_wait(&printState);
}

void AvailabilityReportingFunction(Ivy &bus, unsigned long sent,unsigned long lost,unsigned long received,double avRtt)
{
    BOOST_LOG_TRIVIAL(info) << "\n----------------------------------------------\n"
                            << "AvailabilityMode report\n"
                            << "\tSent " << sent << " bytes\n"
                            << "\tReceived " << received << " bytes\n"
                            << "\tLost " << lost << " bytes\n"
                            << "\tAvRTT " << avRtt * 1000 << " ms\n"
                            << "----------------------------------------------" << std::endl;
    bus.SendMsg("ground AVAIL_REPORT %ld %ld %ld %g",sent,lost,received,1000*avRtt);
}

void ThroughputReportingFunction(Ivy &bus, bool valid,uint32_t received, uint32_t usecs)
{
    double sec = usecs / 1000000.0;
    BOOST_LOG_TRIVIAL(info) << "\n----------------------------------------------" << std::endl
                            << "ThroughputMode report" << std::endl
                            << "\tReceived " << received << " Bytes in " << sec << " s" << std::endl
                            << "\tThroughput " << received / sec << " Bytes/s" << std::endl
                            << "\tThroughput " << 8 * received / sec << " b/s" << std::endl
                            << "----------------------------------------------" << std::endl;
    bus.SendMsg("ground THROUGHPUT_REPORT %d %d",received,usecs);
}

void runBoostMainLoop(io_service &io)
{
    BOOST_LOG_TRIVIAL(info) << "In thread\n";

    unsigned long res = 1;

    while (res)
    {
        res = io.run_one();
    }
}

using std::chrono::seconds;

int main()
{
    setLogLevel(logging::trivial::severity_level::trace);
    try
    {
        io_service io;

        Ivy bus("ModemTester","Ready",new IvyApplicationNullCallback(),0);

        bus.start("127.255.255.255:2010");

        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        using std::placeholders::_4;

        MasterPingerFactory::setPingStatusPrintPeriod(seconds(1));
        MasterPingerFactory::setReportingFunction(std::bind(AvailabilityReportingFunction,bus,_1,_2,_3,_4));
        MasterThroughputFactory::setThroughputResultPrintPeriod(seconds(0));
        MasterThroughputFactory::setReportingFunction(std::bind(ThroughputReportingFunction,bus,_1,_2,_3));
        MasterThroughputFactory::setEstimatedThroughput(5000);

        EventsScript<MasterFSM> eventsScript(io, true);
        auto deadline = seconds(0);
        eventsScript[deadline += seconds(1)] = AvailabilityModeCommand_id;
        eventsScript[deadline += seconds(20)] = EndModeCommand_id;
        eventsScript[deadline += seconds(2)] = ThroughputModeCommand_id;
        eventsScript[deadline += seconds(5)] = EndModeCommand_id;
        eventsScript[deadline += seconds(2)] = EndTest_id;

        tcp::socket socket(io);
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 2020));
        boost::system::error_code ec;
        acceptor.accept(socket, ec);

        AISOBase *ptr = AISOBase::CreateFromStream(socket);

        MasterFSM::setIoStream(ptr);
        MasterFSM::start();

        eventsScript.startScript();

        BOOST_LOG_TRIVIAL(info) << "Starting print timer\n";
        printTimer = new boost::asio::basic_waitable_timer<TheClock>(ptr->get_io_service(), printStatusPeriod);
        printTimer->async_wait(&printState);

        BOOST_LOG_TRIVIAL(info) << "Starting thread\n";
        std::thread thr(std::bind(&runBoostMainLoop,std::ref(io)));

        BOOST_LOG_TRIVIAL(info) << "Starting ivy main loop\n";
        bus.ivyMainLoop();
        bus.stop();
    }
    catch (std::exception &e)
    {
        BOOST_LOG_TRIVIAL(error) << "Error in main loop : " << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}