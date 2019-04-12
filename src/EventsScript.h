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

#ifndef MODEMTESTER_EVENTSSCRIPT_H
#define MODEMTESTER_EVENTSSCRIPT_H

#include "Events.h"
#include "time_def.h"
#include "RunnablePool.h"
#include <map>
#include <iostream>
#include <boost/asio.hpp>
#include "DispatchingRunnable.h"

template<typename FSM, typename ClockType = TheClock>
class EventsScript : public std::map<typename ClockType::duration,EventId_t> {
public:
    // If cyclic, the last event will be replaced by the first
    explicit EventsScript (boost::asio::io_service &io, bool cyclic = false);

    void startScript();
    void stopScript();

    void handleTimer(boost::system::error_code const & ec,EventId_t const &id);

protected:
    template<typename E>
    void dispatchInThread();

    using BaseMapType = std::map<typename ClockType::duration,EventId_t>;
    void restartScript (boost::system::error_code const &ec);
    using timer = boost::asio::basic_waitable_timer<TheClock>;
    boost::asio::io_service &_io;
    void dispatchEventFromId(EventId_t const & id);
    std::map<typename ClockType::time_point,timer*> _timers;
    bool _cyclic;
    RunnablePool *_runPool;
};

template<typename FSM, typename ClockType>
template<typename E>
void EventsScript<FSM, ClockType>::dispatchInThread ()
{
    _runPool->addAndStartRunnable(new DispatchingRunnable<MasterFSM,E>());
}

template<typename FSM, typename DurationType>
void EventsScript<FSM,DurationType>::dispatchEventFromId (EventId_t const &id)
{
    switch (id)
    {
        case Timeout_id:
            dispatchInThread<Timeout>();
            break;
        case AvailabilityModeCommand_id:
            dispatchInThread<AvailabilityModeCommand>();
            break;
        case ThroughputModeCommand_id:
            dispatchInThread<ThroughputModeCommand>();
            break;
        case EndModeCommand_id:
            dispatchInThread<EndModeCommand>();
            break;
        case RemoteAck_id:
            dispatchInThread<RemoteAck>();
            break;
        case MasterAck_id:
            dispatchInThread<MasterAck>();
            break;
        case EndTest_id:
            dispatchInThread<EndTest>();
        break;
        default:
            break;
    }
}

template<typename FSM, typename DurationType>
EventsScript<FSM,DurationType>::EventsScript (boost::asio::io_service &io, bool cyclic):_io(io),_cyclic(cyclic)
{
    _runPool=RunnablePool::getInstance();
}

using std::placeholders::_1;

template<typename FSM, typename ClockType>
void EventsScript<FSM,ClockType>::startScript ()
{
    auto begin = ClockType::now();

    // Clear all timers to avoid memory leak
    stopScript();

    auto lastDate = begin + std::prev(this->end())->first;

    for (auto pair : *this)
    {
        auto date = begin + pair.first;
        auto *t = new timer(_io);
        _timers[date] = t;
        auto &event = pair.second;
        _timers[date]->expires_at(date);
        if (_cyclic && date == lastDate)
        {
            _timers[lastDate]->async_wait(std::bind(
                &EventsScript<FSM, ClockType>::restartScript,
                this,
                _1)
            );
        }
        else
        {
            _timers[date]->async_wait(std::bind(
                &EventsScript<FSM, ClockType>::handleTimer,
                this,
                _1,
                event)
            );
        }
    }
}

template<typename FSM, typename ClockType>
void EventsScript<FSM,ClockType>::stopScript ()
{
    for (auto pair : _timers)
    {
        pair.second->cancel();
        delete (pair.second);
    }
    _timers.clear();
}

template<typename FSM, typename ClockType>
void EventsScript<FSM,ClockType>::handleTimer (boost::system::error_code const &ec, EventId_t const &id)
{
    BOOST_LOG_TRIVIAL(info) << "Script dispatching event " << id << std::endl;
    if (!ec)
    {
        dispatchEventFromId(id);
    }
}

template<typename FSM, typename ClockType>
void EventsScript<FSM, ClockType>::restartScript (boost::system::error_code const &ec)
{
    if (!ec)
    {
        BOOST_LOG_TRIVIAL(info) << "Script restarting" << std::endl;
        startScript();
    }
}

#endif //MODEMTESTER_EVENTSSCRIPT_H
