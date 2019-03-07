//
// Created by garciafa on 04/03/19.
//

// TODO Add logging with levels so that messages can be displayed when needed

#ifndef MODEMTESTER_BASEFSM_H
#define MODEMTESTER_BASEFSM_H

#include <cstdlib>
#include <iostream>
#include <functional>
#include <boost/asio.hpp>
#include "tinyfsm.hpp"
#include "Runnable.h"
#include "AISO.h"
#include "TimeLogger.h"
#include "Events.h"

inline void defaultErrorHandler(EventWithId const &evt)
{
    std::cerr << "Error : received " << evt << " in wrong state" << std::endl;
}

template<typename State_t>
class BaseFSM : public  tinyfsm::Fsm<BaseFSM<State_t>> {
public:
    virtual const State_t stateId() const = 0;

    virtual void react(EndTest const &);

    virtual void react(EventWithId const &evt) = 0;

    virtual void entry(void);  /* entry actions in some states */
    virtual void exit(void);  /* exit actions in some states */

    static AISOBase *getIoStream ();
    static void setIoStream (AISOBase *ioStream);

    static const std::function<void (const EventWithId &)> &getWrongEventHandler ();
    static void setWrongEventHandler (const std::function<void (const EventWithId &)> &wrongEventHandler);

    void SendByte(uint8_t byte);

    void HandleReadEvt(boost::system::error_code const &ec,std::size_t bytesTransferred);

    static const TheClock::duration &getTimeout ();
    static void setTimeout (const TheClock::duration &timeout);

protected:
    static std::function<void(EventWithId const &)> _wrongEventHandler;
    static AISOBase* _ioStream;
    static TheClock::duration _timeout;
    std::array<uint8_t, 10> _recvBuf;
};

template<typename State_t, typename RunnableFactory>
class ActivityState : public BaseFSM<State_t> {
public:
    void entry (void) override;
    void exit (void) override;
    Runnable *getRunnable () const;
protected:
    Runnable *_runnable = nullptr;
    RunnableFactory *_factory;
};

template<typename State_t, typename RunnableFactory>
void ActivityState<State_t,RunnableFactory>::entry (void)
{
    if (_runnable==nullptr)
    {
        _runnable = (*_factory)();
        _runnable->start();
    }
    BaseFSM<State_t>::entry();
}

template<typename State_t, typename RunnableFactory>
void ActivityState<State_t,RunnableFactory>::exit (void)
{
    //std::cout << TimeLogger::now().count() << " " << std::this_thread::get_id() << " # Stopping thread" << std::endl;
    _runnable->stop();
    delete(_runnable);
    _runnable= nullptr;
    BaseFSM<State_t>::exit();
}
template<typename State_t, typename RunnableFactory>
Runnable *ActivityState<State_t, RunnableFactory>::getRunnable () const
{
    return _runnable;
}

template<typename State_t>
class TimerBasedState : public BaseFSM<State_t> {
public:
    void entry (void) override;
    void exit (void) override;
    void dispatchTimeout(boost::system::error_code const &ec);
protected:
    void startTimer(TheClock::duration const &d);
    void cancelTimer();
    boost::asio::basic_waitable_timer<TheClock> *_timer= nullptr;
};


using std::placeholders::_1;
using std::placeholders::_2;

template<typename State_t>
std::function<void(EventWithId const &)> BaseFSM<State_t>::_wrongEventHandler = &defaultErrorHandler;

template<typename State_t>
AISOBase* BaseFSM<State_t>::_ioStream = nullptr;

template<typename State_t>
TheClock::duration BaseFSM<State_t>::_timeout=std::chrono::seconds(1);

template<typename State_t>
void BaseFSM<State_t>::entry(void)
{
    //std::cout << TimeLogger::now().count() << " => entry in " << *BaseFSM<State_t>::current_state_ptr << " state\n";
    AISOBase * ioStream = getIoStream();
    assert(ioStream!= nullptr);
    ioStream->async_read_some(boost::asio::buffer(_recvBuf,1),std::bind(&BaseFSM<State_t>::HandleReadEvt,this,_1,_2));
}

template<typename State_t>
void BaseFSM<State_t>::exit(void)
{
    boost::system::error_code ec;

    //std::cout << TimeLogger::now().count() << " <= exit of " << *BaseFSM<State_t>::current_state_ptr << " state\n";
    getIoStream()->cancel(ec);
    if (ec)
    {
        std::cerr << "Error canceling async io on stream \n";
    }
}

template<typename State_t>
AISOBase *BaseFSM<State_t>::getIoStream ()
{
    return _ioStream;
}

template<typename State_t>
void BaseFSM<State_t>::setIoStream (AISOBase *ioStream)
{
    _ioStream = ioStream;
}

template<typename State_t>
const std::function<void (const EventWithId &)> &BaseFSM<State_t>::getWrongEventHandler ()
{
    return _wrongEventHandler;
}

template<typename State_t>
void BaseFSM<State_t>::setWrongEventHandler (const std::function<void (const EventWithId &)> &wrongEventHandler)
{
    BaseFSM<State_t>::_wrongEventHandler = wrongEventHandler;
}

template<typename State_t>
void BaseFSM<State_t>::SendByte (uint8_t byte)
{
    std::array<uint8_t, 1> buf;
    buf[0]=byte;
    getIoStream()->write(boost::asio::buffer(buf, 1));
}

template<typename State_t>
void BaseFSM<State_t>::HandleReadEvt(boost::system::error_code const &ec,std::size_t bytesTransferred)
{
    if (ec)
    {
        if (ec.value()!=125)
        {
            std::cerr << "Error reading from stream : " << ec.message() << std::endl;
            ::exit(EXIT_FAILURE);
        }
    }
    else
    {
        for (size_t i = 0; i < bytesTransferred; ++i)
        {
            if (_recvBuf[i] != '\n')
            {
                //std::cout << TimeLogger::now().count() << " # received " << (EventId_t)_recvBuf[i] << std::endl;
                switch (_recvBuf[i])
                {
                    case AvailabilityModeCommand_id:
                        BaseFSM<State_t>::dispatch(AvailabilityModeCommand());
                        break;
                    case ThroughputModeCommand_id:
                        BaseFSM<State_t>::dispatch(ThroughputModeCommand());
                        break;
                    case EndModeCommand_id:
                        BaseFSM<State_t>::dispatch(EndModeCommand());
                        break;
                    case RemoteAck_id:
                        BaseFSM<State_t>::dispatch(RemoteAck());
                        break;
                    case MasterAck_id:
                        BaseFSM<State_t>::dispatch(MasterAck());
                        break;
                    case EndTest_id:
                        BaseFSM<State_t>::dispatch(EndTest());
                        break;
                    default:
                        // This should not occur...
                        std::cout << TimeLogger::now().count() << " # STRANGE THING OCCURED : Received " << (int)_recvBuf[i] << " while in "
                                  << *BaseFSM<State_t>::current_state_ptr << " state" << std::endl;
                }

            }
        }
    }
}

template<typename State_t>
const TheClock::duration &BaseFSM<State_t>::getTimeout ()
{
    return _timeout;
}

template<typename State_t>
void BaseFSM<State_t>::setTimeout (const TheClock::duration &timeout)
{
    _timeout = timeout;
}

template<typename State_t>
void BaseFSM<State_t>::react (EndTest const &e)
{
    SendByte(EndTest_id);
    std::cout << TimeLogger::now().count() << " # Received an EndTest event, stopping.\n";
    ::exit(EXIT_SUCCESS);
}

template<typename State_t>
void TimerBasedState<State_t>::dispatchTimeout (boost::system::error_code const &ec)
{
    delete (_timer);
    _timer = nullptr;
    if (!ec)
    {
        //std::cout << TimeLogger::now().count() << " # Timeout occured\n";
        BaseFSM<State_t>::dispatch(Timeout());
    }
}

template<typename State_t>
void TimerBasedState<State_t>::startTimer (TheClock::duration const &d)
{
    if (_timer==nullptr)
    {
        //std::cout << TimeLogger::now().count() << " # starting timer\n";
        _timer = new boost::asio::basic_waitable_timer<TheClock>(BaseFSM<State_t>::getIoStream()->get_io_service(), d);
        _timer->async_wait(std::bind(&TimerBasedState<State_t>::dispatchTimeout, this, _1));
    }
}

template<typename State_t>
void TimerBasedState<State_t>::cancelTimer()
{
    if (_timer!= nullptr)
    {
        //std::cout << TimeLogger::now().count() << " # Canceling timer\n";
        _timer->cancel();
        delete(_timer);
        _timer= nullptr;
    }
}

template<typename State_t>
void TimerBasedState<State_t>::entry (void)
{
    BaseFSM<State_t>::entry();
    startTimer(BaseFSM<State_t>::_timeout);
}

template<typename State_t>
void TimerBasedState<State_t>::exit (void)
{
    cancelTimer();
    BaseFSM<State_t>::exit();
}

#endif //MODEMTESTER_BASEFSM_H
