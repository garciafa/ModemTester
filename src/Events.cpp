//
// Created by garciafa on 04/03/19.
//
#include <iostream>
#include "Events.h"

std::ostream & operator<<(std::ostream& o, EventId_t const &event_id)
{
    switch (event_id)
    {
        case RemoteAck_id:
            o << "RemoteAck";
            break;
        case Timeout_id:
            o << "Timeout";
            break;
        case EndTest_id:
            o << "EndTest";
            break;
        case AvailabilityModeCommand_id:
            o << "AvailabilityModeCommand";
            break;
        case ThroughputModeCommand_id:
            o << "ThroughputModeCommand";
            break;
        case MasterAck_id:
            o << "MasterAck";
            break;
        case EndModeCommand_id:
            o << "EndModeCommand";
            break;
        default:
            o << "Not an event ('" << (int)event_id << "')";
            break;
    }
    return o;
}

std::ostream & operator<<(std::ostream& o, EventWithId const &event)
{
    o << event.eventId();
    return o;
}