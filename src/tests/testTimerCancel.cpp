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

#include <cstdlib>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "../TimeLogger.h"

using namespace boost::asio;
using namespace std::chrono;
using timer = basic_waitable_timer<std::chrono::high_resolution_clock>;

timer *t = nullptr;

void handleTimeout(boost::system::error_code const & ec)
{
    if (ec)
    {
        std::cout << TimeLogger::now().count() << " => " << ec.message() << std::endl;
    }
    else
    {
        std::cout << TimeLogger::now().count() << " => timeout !\n";
        t->expires_from_now(seconds(2));
        t->async_wait(&handleTimeout);
    }
}

void runIoThread(io_service & io)
{
    unsigned long res=1;
    while (res)
    {
        res=io.run_one();
    }
}

int main()
{
    io_service io;

    t = new timer(io,seconds(2));

    t->async_wait(&handleTimeout);

    std::thread thr(std::bind(&runIoThread,std::ref(io)));

    sleep(3);

    t->cancel();

    thr.join();

    return EXIT_SUCCESS;
}