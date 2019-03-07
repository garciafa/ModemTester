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

#include "../AISO.h"
#include <cstdlib>
#include <iostream>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>

using boost::asio::io_service;

using boost::asio::ip::tcp;

boost::array<uint8_t, 128> buf;
AsyncIOBase * ptr;

void handleWrite (boost::system::error_code const &ec,std::size_t bytesTransferred)
{
    if (!ec)
    {
        std::cout << "Successfully sent " << bytesTransferred << " bytes." << std::endl;
    }
    else
    {
        std::cerr << ec << std::endl;
    }
}

void handleRead (boost::system::error_code const &ec,std::size_t bytesTransferred)
{
    if (!ec)
    {
        std::cout << "Received " << bytesTransferred << " bytes : "<<  std::endl;

        for (std::size_t i=0;i<bytesTransferred;++i)
        {
            std::cout << buf[i] << " ";
        }
        std::cout << std::endl;
        ptr->async_write_some(boost::asio::buffer(buf,bytesTransferred),handleWrite);
    }
    else
    {
        std::cerr << ec << std::endl;
    }
}

int main()
{
    try {
        io_service io;
        tcp::socket socket(io);
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 2020));
        boost::system::error_code ec;
        acceptor.accept(socket,ec);

        //socket.async_read_some(boost::asio::buffer(buf),handleRead);
        ptr = AsyncIOBase::CreateFromStream(socket);

        ptr->async_read_some(boost::asio::buffer(buf),handleRead);

        io.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}