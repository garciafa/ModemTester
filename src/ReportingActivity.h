//
// Created by garciafa on 06/03/19.
//

#ifndef MODEMTESTER_REPORTINGACTIVITY_H
#define MODEMTESTER_REPORTINGACTIVITY_H

#include <chrono>
#include "Runnable.h"
#include "TimeLogger.h"
#include <boost/asio.hpp>

template <typename ReportFunctionType>
class ReportingActivity : public Runnable {
public:
    ReportingActivity (AISOBase *ioStream,const TheClock::duration &pingStatusReportPeriod, ReportFunctionType reportingFunction);

    virtual ~ReportingActivity();

    virtual void handleTimerReportStats (boost::system::error_code const &ec);

    void initReporting();

    void endReporting();

    virtual void makeReport() = 0;

protected:
    AISOBase* _ioStream;
    boost::asio::basic_waitable_timer<TheClock> *_reportStatsTimer;
    TheClock::duration _pingStatusReportPeriod;
    ReportFunctionType _reportingFunction;
};

template <typename ReportFunctionType>
ReportingActivity<ReportFunctionType>::ReportingActivity (AISOBase *ioStream,const TheClock::duration &pingStatusReportPeriod, ReportFunctionType reportingFunction)
    : _ioStream(ioStream), _reportStatsTimer(), _pingStatusReportPeriod(pingStatusReportPeriod), _reportingFunction(reportingFunction)
{}

template <typename ReportFunctionType>
void ReportingActivity<ReportFunctionType>::handleTimerReportStats (boost::system::error_code const &ec)
{
    using std::placeholders::_1;
    if (!ec)
    {
        makeReport();
        _reportStatsTimer->expires_from_now(_pingStatusReportPeriod);
        _reportStatsTimer->async_wait(std::bind(&ReportingActivity<ReportFunctionType>::handleTimerReportStats,this,_1));
    }
}


template<typename ReportFunctionType>
void ReportingActivity<ReportFunctionType>::initReporting()
{
    using std::placeholders::_1;
    if (_pingStatusReportPeriod > std::chrono::seconds(0))
    {
        _reportStatsTimer = new boost::asio::basic_waitable_timer<TheClock>(_ioStream->get_io_service(), _pingStatusReportPeriod);
        _reportStatsTimer->async_wait(std::bind(&ReportingActivity<ReportFunctionType>::handleTimerReportStats, this, _1));
    }
}

template<typename ReportFunctionType>
ReportingActivity<ReportFunctionType>::~ReportingActivity ()
{
    if (_reportStatsTimer!= nullptr)
    {
        delete _reportStatsTimer;
        _reportStatsTimer= nullptr;
    }
}

template<typename ReportFunctionType>
void ReportingActivity<ReportFunctionType>::endReporting ()
{
    if (_reportStatsTimer!= nullptr)
        _reportStatsTimer->cancel();
}

#endif //MODEMTESTER_REPORTINGACTIVITY_H
