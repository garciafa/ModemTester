//
// Created by garciafa on 05/03/19.
//

#ifndef MODEMTESTER_EVENTSSCRIPT_H
#define MODEMTESTER_EVENTSSCRIPT_H

#include "Events.h"
#include "TimeLogger.h"
#include "RunnablePool.h"
#include <map>
#include <iostream>
#include <boost/asio.hpp>

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
    RunnablePool _runPool;
};

template <typename E>
class DispatchingRunnable : public Runnable {
public:
    void operator()() override
    {
        E e;
        MasterFSM::dispatch(E());
    }
};

template<typename FSM, typename ClockType>
template<typename E>
void EventsScript<FSM, ClockType>::dispatchInThread ()
{
    _runPool.addAndStartRunnable(new DispatchingRunnable<E>());
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
EventsScript<FSM,DurationType>::EventsScript (boost::asio::io_service &io, bool cyclic):_io(io),_cyclic(cyclic),_runPool() {}

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
    std::cout << TimeLogger::now().count() << " ===> Script dispatching event " << id << std::endl;
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
        std::cout << TimeLogger::now().count() << " # Script restarting" << std::endl;
        startScript();
    }
}

#endif //MODEMTESTER_EVENTSSCRIPT_H
