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
#include "../old/modem_measures.h"
#include "TimeLogger.h"
using namespace boost::asio;

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

    io_service ioService;
    serial_port serialPort(ioService);

    serialPort.open(serialPath);

    if (isRemote)
    {
        remotePing(serialPort);
        //remoteThroughput(serialPort);
    }
    else
    {
        masterPing(serialPort,1,std::chrono::milliseconds(100));
        //masterThroughput(serialPort);
    }

    serialPort.close();

    return EXIT_SUCCESS;
}