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

