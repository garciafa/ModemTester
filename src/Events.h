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

#ifndef MODEMTESTER_COMMANDS_H
#define MODEMTESTER_COMMANDS_H
#include "tinyfsm.hpp"

using  EventId_t = enum e_EventId {
    Timeout_id = -1,
    BeginStandardCodes=0,
    EndStandardCodes=240,
    AvailabilityModeCommand_id,
    ThroughputModeCommand_id,
    EndModeCommand_id,
    RemoteAck_id,
    MasterAck_id,
    EndTest_id,
};

/*
    using  EventId_t = enum e_EventId {
    Timeout_id = -1,
    BeginStandardCodes=0,
    EndStandardCodes='a'-1,
    AvailabilityModeCommand_id='a',
    ThroughputModeCommand_id='t',
    EndModeCommand_id='e',
    RemoteAck_id='r',
    MasterAck_id='m',
    EndTest_id='q',
};
*/
/*
 * Events declarations
 */

struct EventWithId : tinyfsm::Event {
    virtual const EventId_t eventId() const = 0;
};

std::ostream & operator<<(std::ostream& o, EventWithId const &event);
std::ostream & operator<<(std::ostream& o, EventId_t const &event_id);

struct AvailabilityModeCommand : EventWithId {
    const EventId_t eventId() const override {return AvailabilityModeCommand_id;}
};
struct ThroughputModeCommand : EventWithId {
    const EventId_t eventId() const override {return ThroughputModeCommand_id;}
};
struct EndModeCommand : EventWithId {
    const EventId_t eventId() const override {return EndModeCommand_id;}
};
struct RemoteAck : EventWithId {
    const EventId_t eventId() const override {return RemoteAck_id;}
};
struct MasterAck : EventWithId {
    const EventId_t eventId() const override {return MasterAck_id;}
};
struct Timeout : EventWithId {
    const EventId_t eventId() const override {return Timeout_id;}
};
struct EndTest : EventWithId {
    const EventId_t eventId() const override {return EndTest_id;}
};
#endif //MODEMTESTER_COMMANDS_H
