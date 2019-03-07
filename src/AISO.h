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

#ifndef MODEMTESTER_AISO_H
#define MODEMTESTER_AISO_H

#include <boost/asio.hpp>
#include <functional>

using Handler = std::function<void(boost::system::error_code const&, std::size_t)>;

using boost::asio::io_service;

// Asynchronous Input / Synchronous Output abstraction
class AISOBase {
public:
    template<typename AsyncReadWriteStream>
        static AISOBase* CreateFromStream(AsyncReadWriteStream &stream);

    virtual io_service& get_io_service() = 0;

    virtual void async_read_some(boost::asio::mutable_buffers_1 mb,Handler handler) = 0;

    virtual void async_read(boost::asio::mutable_buffers_1 mb,Handler handler) = 0;

    virtual void write_some (boost::asio::mutable_buffers_1 mb) = 0;

    virtual void write (boost::asio::mutable_buffers_1 mb) = 0;

    virtual void cancel() = 0;

    virtual void cancel(boost::system::error_code &ec) = 0;
};

// Asynchronous Input / Synchronous Output for boost::asio
template<typename AsyncReadStream>
class AISO : public AISOBase {
public:
    AISO (AsyncReadStream &stream);
    io_service& get_io_service() override final;

    void async_read_some(boost::asio::mutable_buffers_1 mb,Handler handler) override final;

    void async_read(boost::asio::mutable_buffers_1 mb,Handler handler) override final;

    void write_some (boost::asio::mutable_buffers_1 mb) override final;

    void write (boost::asio::mutable_buffers_1 mb) override final;

    void cancel() override final;

    void cancel(boost::system::error_code &ec) override final;
private:
    AsyncReadStream &_stream;
};

template<typename AsyncReadWriteStream>
AISOBase *AISOBase::CreateFromStream (AsyncReadWriteStream &stream)
{
    return dynamic_cast<AISOBase *>(new AISO<AsyncReadWriteStream>(stream));
}

template<typename AsyncReadStream>
AISO<AsyncReadStream>::AISO (AsyncReadStream &stream):_stream(stream){}

template<typename AsyncReadStream>
io_service &AISO<AsyncReadStream>::get_io_service ()
{
    return _stream.get_io_service();
}

template<typename AsyncReadStream>
void AISO<AsyncReadStream>::async_read_some (boost::asio::mutable_buffers_1 mb, Handler handler)
{
    _stream.async_read_some(mb,handler);
}

template<typename AsyncReadStream>
void AISO<AsyncReadStream>::cancel ()
{
    _stream.cancel();
}

template<typename AsyncReadStream>
void AISO<AsyncReadStream>::cancel (boost::system::error_code &ec)
{
    _stream.cancel(ec);
}

template<typename AsyncReadStream>
void AISO<AsyncReadStream>::write_some (boost::asio::mutable_buffers_1 mb)
{
    _stream.write_some(mb);
}

template<typename AsyncReadStream>
void AISO<AsyncReadStream>::async_read (boost::asio::mutable_buffers_1 mb, Handler handler)
{
    boost::asio::async_read(_stream,mb,handler);
}

template<typename AsyncReadStream>
void AISO<AsyncReadStream>::write (boost::asio::mutable_buffers_1 mb)
{
    boost::asio::write(_stream,mb);
}

#endif //MODEMTESTER_AISO_H
