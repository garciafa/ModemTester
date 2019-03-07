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

#ifndef MODEMTESTER_REMOTEFSM_H
#define MODEMTESTER_REMOTEFSM_H
#include "../BaseFSM.h"
#include "RemotePinger.h"
#include "RemoteThroughput.h"

using  RemoteStateId_t = enum e_RemoteStateId {
    RemoteIdle_id,
    AvailabilityCommandReceived_id,
    ThroughputCommandReceived_id,
    RemoteAvailabilityMode_id,
    RemoteThroughputMode_id,
    EndModeReceived_id
};

using RemoteFSM = BaseFSM<RemoteStateId_t>;

// Stream output operators
std::ostream & operator<<(std::ostream& o, RemoteFSM const &state);

/*
 * States declarations
 */
class RemoteIdle : public RemoteFSM {
public:
    const RemoteStateId_t stateId() const override {return RemoteIdle_id;}
    void react (EventWithId const &evt) override;
};

class AvailabilityCommandReceived : public TimerBasedState<RemoteStateId_t> {
public:
    const RemoteStateId_t stateId() const override {return AvailabilityCommandReceived_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
};

class ThroughputCommandReceived : public TimerBasedState<RemoteStateId_t> {
public:
    const RemoteStateId_t stateId() const override {return ThroughputCommandReceived_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
};

class RemoteAvailabilityMode : public ActivityState<RemoteStateId_t,RemotePingerFactory> {
public:
    const RemoteStateId_t stateId() const override {return RemoteAvailabilityMode_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
};

class RemoteThroughputMode : public ActivityState<RemoteStateId_t,RemoteThroughputFactory> {
public:
    const RemoteStateId_t stateId() const override {return RemoteThroughputMode_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
};

class EndModeReceived : public TimerBasedState<RemoteStateId_t> {
public:
    const RemoteStateId_t stateId() const override {return EndModeReceived_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
};

#endif //MODEMTESTER_REMOTEFSM_H
