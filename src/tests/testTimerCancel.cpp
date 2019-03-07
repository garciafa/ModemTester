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
        std::cout << ec.message() << std::endl;
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

    sleep(1);

    t->cancel();

    thr.join();

    return EXIT_SUCCESS;
}