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

#include "RemoteFSM.h"

std::ostream & operator<<(std::ostream& o, RemoteFSM const &state)
{
    switch (state.stateId())
    {
        case RemoteIdle_id:
            o << "RemoteIdle";
            break;
        case AvailabilityCommandReceived_id:
            o << "AvailabilityCommandReceived";
            break;
        case ThroughputCommandReceived_id:
            o << "ThroughputCommandReceived";
            break;
        case RemoteAvailabilityMode_id:
            o << "RemoteAvailabilityMode";
            break;
        case RemoteThroughputMode_id:
            o << "RemoteThroughputMode";
            break;
        case EndModeReceived_id:
            o << "EndModeReceived";
            break;
    }
    return o;
}

void RemoteErrorHandler(EventWithId const &evt)
{
    BOOST_LOG_TRIVIAL(error) << "Error : received " << evt << " in state " << *RemoteFSM::current_state_ptr << std::endl;
}

template<>
std::function<void(EventWithId const &)> RemoteFSM::_wrongEventHandler = &RemoteErrorHandler;

void RemoteIdle::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case AvailabilityModeCommand_id:
            transit<AvailabilityCommandReceived>();
            break;
        case ThroughputModeCommand_id:
            transit<ThroughputCommandReceived>();
            break;
        default:
            // Ignore all other events
            break;
    }
}

void AvailabilityCommandReceived::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case MasterAck_id:
            transit<RemoteAvailabilityMode>();
            break;
        case Timeout_id:
            transit<AvailabilityCommandReceived>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}

void AvailabilityCommandReceived::entry (void)
{
    BOOST_LOG_TRIVIAL(trace) << "Sending RACK" << std::endl;
    SendByte(RemoteAck_id);
    TimerBasedState<RemoteStateId_t>::entry();
}

void ThroughputCommandReceived::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case MasterAck_id:
            transit<RemoteThroughputMode>();
            break;
        case Timeout_id:
            transit<ThroughputCommandReceived>();
            break;
        default:
            // Ignore all other events
            break;
    }
}

void ThroughputCommandReceived::entry (void)
{
    BOOST_LOG_TRIVIAL(trace) << "Sending RACK" << std::endl; 
    SendByte(RemoteAck_id);
    TimerBasedState<RemoteStateId_t>::entry();
}

void RemoteAvailabilityMode::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case EndModeCommand_id:
            transit<EndModeReceived>();
            break;
        case MasterAck_id:
            transit<RemoteAvailabilityMode>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}

void RemoteAvailabilityMode::entry (void)
{
    AISOBase * ioStream = getIoStream();
    assert(ioStream!= nullptr);
    _factory = new RemotePingerFactory(ioStream);
    ActivityState<RemoteStateId_t,RemotePingerFactory>::entry();
}

void RemoteThroughputMode::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case EndModeCommand_id:
            transit<EndModeReceived>();
            break;
        case MasterAck_id:
            transit<RemoteThroughputMode>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}

void RemoteThroughputMode::entry (void)
{
    AISOBase * ioStream = getIoStream();
    assert(ioStream!= nullptr);
    _factory = new RemoteThroughputFactory(ioStream);
    ActivityState<RemoteStateId_t,RemoteThroughputFactory>::entry();
}

void EndModeReceived::react (EventWithId const &evt)
{
    switch (evt.eventId())
    {
        case EndModeCommand_id:
        case Timeout_id:
            transit<EndModeReceived>();
            break;
        case MasterAck_id:
            transit<RemoteIdle>();
            break;
        default:
            _wrongEventHandler(evt);
            break;
    }
}

void EndModeReceived::entry (void)
{
    BOOST_LOG_TRIVIAL(trace) << "Sending RACK" << std::endl; 
    SendByte(RemoteAck_id);
    TimerBasedState<RemoteStateId_t>::entry();
}
