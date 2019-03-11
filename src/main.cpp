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
#include "TimeLogger.h"
#include "AISO.h"
#include "EventsScript.h"

using namespace boost::asio;

FSM_INITIAL_STATE(RemoteFSM,RemoteIdle);
FSM_INITIAL_STATE(MasterFSM,Idle);

void ThroughputReportingFunction(bool valid,uint32_t received, uint32_t usecs);
void AvailabilityReportingFunction(unsigned long sent,unsigned long lost,unsigned long received,double avRtt);

int main (int argc,char** argv)
{
    std::string serialPath;
    bool isRemote;

    // Declare the supported options.
    boost::program_options::options_description desc("Options ");
    boost::program_options::positional_options_description p;
    desc.add_options()
        ("help,h", "help message")
        ("remote,r", "Act as remote end (default: act as master)");


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

    io_service io;
    serial_port serialPort(io);

    serialPort.open(serialPath);

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
            MasterPingerFactory::setPingStatusPrintPeriod(seconds(0));
            MasterPingerFactory::setReportingFunction(AvailabilityReportingFunction);
            MasterThroughputFactory::setPingStatusPrintPeriod(seconds(0));
            MasterThroughputFactory::setReportingFunction(ThroughputReportingFunction);

            // TODO Read the script from a file
            EventsScript<MasterFSM> eventsScript(io,true);
            auto deadline = seconds(0);
            eventsScript[deadline+=seconds(1)] = AvailabilityModeCommand_id;
            eventsScript[deadline+=seconds(2)] = EndModeCommand_id;
            eventsScript[deadline+=seconds(1)] = ThroughputModeCommand_id;
            eventsScript[deadline+=seconds(2)] = EndModeCommand_id;
            eventsScript[deadline+=seconds(5)] = EndTest_id;

            AISOBase *ptr = AISOBase::CreateFromStream(serialPort);

            TimeLogger::reset();
            MasterFSM::setIoStream(ptr);
            MasterFSM::start();
            eventsScript.startScript();

            unsigned long res=1;

            while (res)
            {
                res = io.run_one();
            }
        }
        catch (std::exception &e)
        {
            std::cerr << "Error in main loop : " << e.what() << std::endl;
        }
    }

    serialPort.close();

    return EXIT_SUCCESS;
}

void AvailabilityReportingFunction(unsigned long sent,unsigned long lost,unsigned long received,double avRtt)
{
    std::cout << "----------------------------------------------" << std::endl;
    std::cout << TimeLogger::now().count() << " # AvailabilityMode report" << std::endl;
    std::cout << "\tSent " << sent << " bytes\n";
    std::cout << "\tReceived " << received << " bytes\n";
    std::cout << "\tLost " << lost << " bytes\n";
    std::cout << "\tAvRTT " << avRtt*1000 << " ms\n";
    std::cout << "----------------------------------------------" << std::endl;
}

void ThroughputReportingFunction(bool valid,uint32_t received, uint32_t usecs)
{
    double sec = usecs / 1000000.0;
    std::cout << "----------------------------------------------" << std::endl;
    std::cout << TimeLogger::now().count() << " # ThroughputMode report" << std::endl;
    std::cout << "\tReceived " << received << " Bytes in " << sec << " s" << std::endl;
    std::cout << "\tThroughput " << received / sec << " Bytes/s" << std::endl;
    std::cout << "\tThroughput " << 8 * received / sec << " b/s" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;

}