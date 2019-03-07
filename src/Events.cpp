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