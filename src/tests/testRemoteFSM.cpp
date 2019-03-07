//
// Created by garciafa on 04/03/19.
//

//
// Created by garciafa on 28/02/19.
//

#include <cstdlib>
#include <iostream>
#include <functional>
#include <boost/asio.hpp>
#include "../remote/RemoteFSM.h"
#include "../TimeLogger.h"

FSM_INITIAL_STATE(RemoteFSM,RemoteIdle);

using boost::asio::io_service;

using boost::asio::ip::tcp;

boost::asio::basic_waitable_timer<TheClock> *printTimer;

void printState(boost::system::error_code const & ec)
{
    std::cout << TimeLogger::now().count() << " # Current state : " << *RemoteFSM::current_state_ptr << std::endl;
    printTimer->expires_from_now(std::chrono::seconds(2));
    printTimer->async_wait(&printState);
}

int main()
{
    try
    {
        io_service io;
        tcp::socket socket(io);
        tcp::endpoint ep(tcp::v4(), 2020);
        boost::system::error_code ec;

        socket.connect(ep);

        AISOBase *ptr = AISOBase::CreateFromStream(socket);

        RemoteFSM::setIoStream(ptr);
        RemoteFSM::start();

        //unsigned long res=1;
        printTimer = new boost::asio::basic_waitable_timer<TheClock>(ptr->get_io_service(), std::chrono::milliseconds(500));
        printTimer->async_wait(&printState);

        /*
    while (res)
    {
        res = io.run_one();
        if (!res)
        {
            std::cout << "No handler executed => connection lost or no more handler registered\n";
        }
        }
         */
        io.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error in main loop : " << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}