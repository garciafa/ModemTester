//
// Created by garciafa on 28/02/19.
//

#include "MasterFSM.h"
#include <iostream>
#include <boost/asio/io_service.hpp>
#include "MasterPinger.h"

void MasterErrorHandler(EventWithId const &evt)
{
    std::cerr << "Error : received " << evt << " in state" << *MasterFSM::current_state_ptr << std::endl;
}

template<>
std::function<void(EventWithId const &)> MasterFSM::_wrongEventHandler = &MasterErrorHandler;

void Idle::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case AvailabilityModeCommand_id:
            transit<SentAvailableModeCommand>();
            break;
        case ThroughputModeCommand_id:
            transit<SentThroughputModeCommand>();
            break;
        case RemoteAck_id:
            //std::cout << TimeLogger::now().count() << " # Sending MACK" << std::endl;
            SendByte(MasterAck_id);
            transit<Idle>();
            break;
        default:
            // Ignore all other events
            break;
    }
}

void SentAvailableModeCommand::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case Timeout_id:
            transit<SentAvailableModeCommand>();
            break;
        case RemoteAck_id:
            transit<AvailabilityMode>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}

void SentAvailableModeCommand::entry (void)
{
    //std::cout << TimeLogger::now().count() << " # Sending AvailableModeCommand" << std::endl;
    SendByte(AvailabilityModeCommand_id);
    startTimer(_timeout);
    TimerBasedState<MasterStateId_t>::entry();
}

void SentThroughputModeCommand::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case Timeout_id:
            transit<SentThroughputModeCommand>();
            break;
        case RemoteAck_id:
            transit<ThroughputMode>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}

void SentThroughputModeCommand::entry (void)
{
    startTimer(_timeout);
    //std::cout << TimeLogger::now().count() << " # Sending ThroughputModeCommand" << std::endl;
    SendByte(ThroughputModeCommand_id);
    TimerBasedState<MasterStateId_t>::entry();
}

void AvailabilityMode::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case EndModeCommand_id:
            transit<WaitAckAfterEndMode>();
            break;
        case RemoteAck_id:
            transit<AvailabilityMode>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}

void AvailabilityMode::entry (void)
{
    //std::cout << TimeLogger::now().count() << " # Sending MACK" << std::endl;
    SendByte(MasterAck_id);
    AISOBase * ioStream = getIoStream();
    assert(ioStream!= nullptr);
    if (_factory== nullptr)
        _factory = new MasterPingerFactory(ioStream);
    ActivityState<MasterStateId_t,MasterPingerFactory>::entry();
}

void ThroughputMode::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case EndModeCommand_id:
            transit<WaitAckAfterEndMode>();
            break;
        case RemoteAck_id:
            transit<ThroughputMode>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}
void ThroughputMode::entry (void)
{
    //std::cout << TimeLogger::now().count() << " # Sending MACK" << std::endl;
    SendByte(MasterAck_id);
    AISOBase *ioStream = getIoStream();
    assert(ioStream != nullptr);
    if (_factory== nullptr)
        _factory = new MasterThroughputFactory(ioStream);
    ActivityState<MasterStateId_t, MasterThroughputFactory>::entry();
}

void WaitAckAfterEndMode::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case RemoteAck_id:
            //std::cout << TimeLogger::now().count() << " # Sending MACK" << std::endl;
            SendByte(MasterAck_id);
            transit<Idle>();
            break;
        case Timeout_id:
            transit<WaitAckAfterEndMode>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}

void WaitAckAfterEndMode::entry (void)
{
    //std::cout << TimeLogger::now().count() << " # Sending EndModeCommand" << std::endl;
    SendByte(EndModeCommand_id);
    TimerBasedState<MasterStateId_t>::entry();
}

std::ostream & operator<<(std::ostream& o, MasterFSM const &state)
{
    switch (state.stateId())
    {
        case Idle_id:
            o << "Idle";
            break;
        case SentAvailableModeCommand_id:
            o << "SentAvailableModeCommand";
            break;
        case SentThroughputModeCommand_id:
            o << "SentThroughputModeCommand";
            break;
        case AvailabilityMode_id:
            o << "AvailabilityMode";
            break;
        case ThroughputMode_id:
            o << "ThroughputMode";
            break;
        case WaitAckAfterEndMode_id:
            o << "WaitAckAfterEndMode";
            break;
    }
    return o;
}
