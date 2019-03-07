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
// Created by garciafa on 28/02/19.
//

#ifndef MODEMTESTER_MASTERTESTER_H
#define MODEMTESTER_MASTERTESTER_H

#include "../BaseFSM.h"
#include "MasterPinger.h"
#include "MasterThroughput.h"

/*
 * FSM class declaration
 */
using  MasterStateId_t = enum e_MasterStateId {
    Idle_id,
    SentAvailableModeCommand_id,
    SentThroughputModeCommand_id,
    AvailabilityMode_id,
    ThroughputMode_id,
    WaitAckAfterEndMode_id,
};

using MasterFSM = BaseFSM<MasterStateId_t>;

// Stream output operators
std::ostream & operator<<(std::ostream& o, MasterFSM const &state);

/*
 * States declarations
 */

class Idle : public MasterFSM {
public:
    const MasterStateId_t stateId() const override {return Idle_id;}
    void react (EventWithId const &evt) override;
};

class SentAvailableModeCommand : public TimerBasedState<MasterStateId_t> {
public:
    const MasterStateId_t stateId() const override {return SentAvailableModeCommand_id;}
    void entry (void) override;
    void react (EventWithId const &evt) override;
};

class SentThroughputModeCommand : public TimerBasedState<MasterStateId_t> {
public:
    const MasterStateId_t stateId() const override {return SentThroughputModeCommand_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
};

class AvailabilityMode : public ActivityState<MasterStateId_t,MasterPingerFactory> {
public:
    const MasterStateId_t stateId() const override {return AvailabilityMode_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
protected:
};

class ThroughputMode : public ActivityState<MasterStateId_t,MasterThroughputFactory> {
public:
    const MasterStateId_t stateId() const override {return ThroughputMode_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
};

class WaitAckAfterEndMode : public TimerBasedState<MasterStateId_t> {
public:
    const MasterStateId_t stateId() const override {return WaitAckAfterEndMode_id;}
    void react (EventWithId const &evt) override;
    void entry (void) override;
};


#endif //MODEMTESTER_MASTERTESTER_H

