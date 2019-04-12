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

#ifndef MODEMTESTER_MASTERTHROUGHPUT_H
#define MODEMTESTER_MASTERTHROUGHPUT_H

#include "../Runnable.h"
#include "../AISO.h"
#include "../CommunicatingRunnableFactory.h"
#include "../Events.h"
#include "../time_def.h"
#include "../ReportingActivity.h"

using ThroughputReportFunctionType = std::function<void(bool,uint32_t,uint32_t)>;

class MasterThroughput : public ReportingActivity<ThroughputReportFunctionType> {
public:
    MasterThroughput (AISOBase *ioStream,
        TheClock::duration pingStatusReportPeriod = std::chrono::seconds(0),
        ThroughputReportFunctionType reportFunction=&noReportingFunction,
        uint32_t estimatedThroughput = 9600);

    ~MasterThroughput () override = default;

    void operator()() override;

    void handleReceive(boost::system::error_code const & ec,std::size_t bytesAvailable);

    void getResults ();
    void makeReport () override;
    void receiveResults (const boost::system::error_code &ec, std::size_t bytesAvailable);
    void timeoutReceiveResults (const boost::system::error_code &ec);

    static void noReportingFunction (bool b,uint32_t r,uint32_t u);

protected:
    std::array<uint8_t, 2*sizeof(uint32_t)> _recvBuf;
    volatile bool _gotResults;
    volatile bool _resultsReady;
    uint32_t _totalReceived;
    uint32_t _totalUsec;
    boost::asio::basic_waitable_timer<TheClock> *_receiveResultsTimer;
    uint32_t _estimatedThroughput; /**< In bits/s */
};

class MasterThroughputFactory : public CommunicatingRunnableFactory<MasterThroughput> {
public:
    explicit MasterThroughputFactory (AISOBase *ioStream);

    static const TheClock::duration &getThroughputResultPrintPeriod ();
    static const uint32_t getEstimatedThroughput ();

    static void setThroughputResultPrintPeriod (const TheClock::duration &throughputResultPrintPeriod);
    static void setReportingFunction (const ThroughputReportFunctionType &reportingFunction);
    static void setEstimatedThroughput (const uint32_t &estimatedThroughput);

    MasterThroughput* operator()() override;
protected:
    static TheClock::duration _throughputResultPrintPeriod;
    static ThroughputReportFunctionType _reportingFunction;
    static uint32_t _estimatedThroughput;
};

#endif //MODEMTESTER_MASTERTHROUGHPUT_H
