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

#include <cstdlib>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include "remote/RemoteFSM.h"
#include "master/MasterFSM.h"
#include "time_def.h"
#include "AISO.h"
#include "EventsScript.h"
#include "master/MasterController.h"
#include <Ivy/Ivycpp.h>

using namespace boost::asio;

FSM_INITIAL_STATE(RemoteFSM,RemoteIdle);
FSM_INITIAL_STATE(MasterFSM,Idle);

void ThroughputReportingFunction(Ivy &bus,MasterController &control,  bool valid,uint32_t received, uint32_t usecs);
void AvailabilityReportingFunction(Ivy &bus,MasterController &control, bool reachable, unsigned long sent,unsigned long lost,unsigned long received,double avRtt);
void SimpleAvailabilityReportingFunction(Ivy &bus, bool reachable, unsigned long sent,unsigned long lost,unsigned long received,double avRtt);

void runBoostMainLoop(io_service &io);

boost::asio::basic_waitable_timer<TheClock> *timer;
void handle (boost::system::error_code const&ec)
{
    timer->expires_from_now(std::chrono::seconds(1));
    timer->async_wait(&handle);
}

int main (int argc,char** argv)
{
    std::string serialPath;
    bool isRemote;
    unsigned int logLevel;
    unsigned int baudRate;
    std::string busAddr;

    // Declare the supported options.
    boost::program_options::options_description desc("Options ");
    boost::program_options::positional_options_description p;
    desc.add_options()
        ("help,h", "help message")
        ("remote", "Act as remote end (default: act as master)")
        ("bus,b", boost::program_options::value<std::string>(&busAddr)->default_value("127.255.255.255:2010"),"Ivy bus to use")
        ("rate,r", boost::program_options::value<unsigned int>(&baudRate)->default_value(57600),"Baudrate of the serial line to the modem")
        ("log_level,l",boost::program_options::value<unsigned int> (&logLevel)->default_value(3),"Log level from 0 (fatal errors) to 5 (traces)");


    boost::program_options::options_description hid ("Positional Options");
    hid.add_options ()
        ("path",boost::program_options::value<std::string> (&serialPath),"Path to serial line");

    p.add("path", 1);

    try
    {
        boost::program_options::options_description cmdLine;
        cmdLine.add(desc).add(hid);

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(cmdLine).positional(p).run(), vm);
        boost::program_options::notify (vm);

        if (vm.count ("help"))
        {
            std::cout << argv[0] << " [options] <serial path>" << std::endl;
            std::cout << desc << std::endl;
            exit(EXIT_SUCCESS);
        }

        isRemote=(vm.count ("remote")>0);

            if (!vm.count ("path"))
        {
            std::cout << "No path provided !" << std::endl;
            std::cout << argv[0] << " [options] <path>" << std::endl;
            std::cout << desc << std::endl;
            exit(EXIT_FAILURE);
        }
        if (logLevel < 0 || logLevel > 5)
        {
            std::cerr << "Error : Logging level can only range from 0 to 5." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    catch (boost::program_options::too_many_positional_options_error &err)
    {
        std::cerr << err.what () << std::endl;
        std::cerr << "Only one path should be provided." << std::endl;
        std::cout << argv[0] << " [options] <path>" << std::endl;
        std::cout << desc << std::endl;
        exit(EXIT_FAILURE);
    }
    catch (boost::program_options::unknown_option &err)
    {
        std::cerr << err.what () << std::endl;
        std::cout << argv[0] << " [options] <path>" << std::endl;
        std::cout << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    setLogLevel(static_cast<logging::trivial::severity_level>(logging::trivial::fatal - logLevel));

    io_service io;
    serial_port serialPort(io);
    serialPort.open(serialPath);
    serialPort.set_option(serial_port::baud_rate(baudRate));

    if (isRemote)
    {
        try
        {
            RemoteFSM::setIoStream(AISOBase::CreateFromStream(serialPort));
            RemoteFSM::start();

            io.run();
        }
        catch (std::exception &e)
        {
            std::cerr << "Error in main loop : " << e.what() << std::endl;
        }
    }
    else
    {
        using std::chrono::seconds;
        try
        {
            Ivy bus("ModemTester","Ready",new IvyApplicationNullCallback(),0);

            bus.start(busAddr.c_str());

            using std::placeholders::_1;
            using std::placeholders::_2;
            using std::placeholders::_3;
            using std::placeholders::_4;
            using std::placeholders::_5;

            AISOBase *ptr = AISOBase::CreateFromStream(serialPort);

            //MasterController control(ptr);

            MasterPingerFactory fact(ptr);
            MasterPingerFactory::setPingStatusPrintPeriod(seconds(1));
            MasterPingerFactory::setReportingFunction(std::bind(SimpleAvailabilityReportingFunction,std::ref(bus),_1,_2,_3,_4,_5));
            /*
            MasterThroughputFactory::setThroughputResultPrintPeriod(seconds(0));
            MasterThroughputFactory::setReportingFunction(std::bind(ThroughputReportingFunction,std::ref(bus),std::ref(control),_1,_2,_3));
            MasterThroughputFactory::setEstimatedThroughput(5000);
            */

            MasterPinger * pinger = fact();

            pinger->start();

            //MasterFSM::setIoStream(ptr);
            //MasterFSM::start();
            //control.start();

            timer = new boost::asio::basic_waitable_timer<TheClock>(io);
            timer->expires_from_now(std::chrono::seconds(1));
            timer->async_wait(&handle);

            std::thread thr(std::bind(&runBoostMainLoop,std::ref(io)));

            bus.ivyMainLoop();

            pinger->stop();
            bus.stop();
            thr.join();
        }
        catch (std::exception &e)
        {
            BOOST_LOG_TRIVIAL(error) << "Error in main loop : " << e.what() << std::endl;
        }
    }

    serialPort.close();

    return EXIT_SUCCESS;
}

void AvailabilityReportingFunction(Ivy &bus, MasterController &control, bool reachable, unsigned long sent,unsigned long lost,unsigned long received,double avRtt)
{
    BOOST_LOG_TRIVIAL(info) << "\n----------------------------------------------\n"
                            << "AvailabilityMode report\n"
                            << "\tReachable " << (reachable?"yes":"no") << "\n"
                            << "\tSent " << sent << " bytes\n"
                            << "\tReceived " << received << " bytes\n"
                            << "\tLost " << lost << " bytes\n"
                            << "\tAvRTT " << avRtt * 1000 << " ms\n"
                            << "----------------------------------------------" << std::endl;
    bus.SendMsg("ground AVAIL_REPORT %d %ld %ld %ld %02.2f",(reachable?1:0),sent,lost,received,avRtt*1000);
    control.setRemoteReachability(reachable);
}

void SimpleAvailabilityReportingFunction(Ivy &bus, bool reachable, unsigned long sent,unsigned long lost,unsigned long received,double avRtt)
{
    BOOST_LOG_TRIVIAL(info) << "\n----------------------------------------------\n"
                            << "AvailabilityMode report\n"
                            << "\tReachable " << (reachable?"yes":"no") << "\n"
                            << "\tSent " << sent << " bytes\n"
                            << "\tReceived " << received << " bytes\n"
                            << "\tLost " << lost << " bytes\n"
                            << "\tAvRTT " << avRtt * 1000 << " ms\n"
                            << "----------------------------------------------" << std::endl;
    bus.SendMsg("ground AVAIL_REPORT %d %ld %ld %ld %02.2f",(reachable?1:0),sent,lost,received,avRtt*1000);
}

void ThroughputReportingFunction(Ivy &bus, MasterController &control, bool valid,uint32_t received, uint32_t usecs)
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
    unsigned long res = 1;

    while (res)
    {
        res = io.run_one();
    }
}
