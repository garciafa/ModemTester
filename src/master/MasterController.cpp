//
// Created by garciafa on 26/03/19.
//

#include "MasterController.h"
#include "../DispatchingRunnable.h"
#include "../RunnablePool.h"

using std::placeholders::_1;

MasterController::MasterController (AISOBase *ioSstream)
: _ioSstream(ioSstream), _timer(ioSstream->get_io_service()), _remoteReachable(false), _tryingAvalability(false)
{}


void MasterController::start ()
{
    _timer.expires_from_now(std::chrono::seconds(1));
    _timer.async_wait(std::bind(&MasterController::periodicAction,this,_1));
}

void MasterController::stop ()
{
    RunnablePool::getInstance()->addAndStartRunnable(new DispatchingRunnable<MasterFSM,EndTest>());
}

void MasterController::periodicAction (boost::system::error_code const & ec)
{
    switch (MasterFSM::current_state_ptr->stateId())
    {
        case Idle_id:
            // If we are currently idle and the remote is not available, try an availability test
            // TODO REMOVE THIS LATER AFTER TESTS
            _remoteReachable = false; // Consider it is not reachable and hence do not do thorughput test
            if (not _remoteReachable)
            {
                // Only dispatch the signal the first time. After that we wait for the FSM to go in availability mode
                if (not _tryingAvalability)
                {
                    _tryingAvalability = true;
                    BOOST_LOG_TRIVIAL(info) << "Starting availability check test" << std::endl;
                    RunnablePool::getInstance()->addAndStartRunnable(new DispatchingRunnable<MasterFSM, AvailabilityModeCommand>());
                }
                _timer.expires_from_now(std::chrono::seconds(1));
                _timer.async_wait(std::bind(&MasterController::periodicAction, this, _1));
            }
            else
            {
                // We already did availability check and remote was available => do a thrgouhput test
                RunnablePool::getInstance()->addAndStartRunnable(new DispatchingRunnable<MasterFSM, ThroughputModeCommand>());
                _timer.expires_from_now(std::chrono::seconds(1));
                _timer.async_wait(std::bind(&MasterController::periodicAction, this, _1));
                _remoteReachable=false; // After throughput we need to test reachability again
            }
            break;
        case AvailabilityMode_id:
            if (_tryingAvalability)
            {
                // We were starting an availability check, do that for 20 seconds
                _tryingAvalability=false;
                BOOST_LOG_TRIVIAL(info) << "Doing availability test for 60 more seconds" << std::endl;
                _timer.expires_from_now(std::chrono::seconds(60));
                _timer.async_wait(std::bind(&MasterController::periodicAction, this, _1));
            }
            else
            {
                // We have done availability test, go to idle
                BOOST_LOG_TRIVIAL(info) << "Ending availability test => going to idle" << std::endl;
                RunnablePool::getInstance()->addAndStartRunnable(new DispatchingRunnable<MasterFSM, EndModeCommand>());
                _timer.expires_from_now(std::chrono::seconds(1));
                _timer.async_wait(std::bind(&MasterController::periodicAction, this, _1));
            }
            break;
        default:
            BOOST_LOG_TRIVIAL(info) << "In state " << *MasterFSM::current_state_ptr << " waiting one more second." << std::endl;
            // In all other cases, just wait one more second
            _timer.expires_from_now(std::chrono::seconds(1));
            _timer.async_wait(std::bind(&MasterController::periodicAction, this, _1));
    }
}

void MasterController::setRemoteReachability (bool reachable)
{
    _remoteReachable=reachable;
}
