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

#ifndef MODEMTESTER_REPORTINGACTIVITY_H
#define MODEMTESTER_REPORTINGACTIVITY_H

#include <chrono>
#include "Runnable.h"
#include "time_def.h"
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
