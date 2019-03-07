//
// Created by garciafa on 04/03/19.
//

#ifndef MODEMTESTER_MASTERTHROUGHPUT_H
#define MODEMTESTER_MASTERTHROUGHPUT_H

#include "../Runnable.h"
#include "../AISO.h"
#include "../CommunicatingRunnableFactory.h"
#include "../Events.h"
#include "../TimeLogger.h"
#include "../ReportingActivity.h"

using ThroughputReportFunctionType = std::function<void(bool,uint32_t,uint32_t)>;

class MasterThroughput : public ReportingActivity<ThroughputReportFunctionType> {
public:
    explicit MasterThroughput (AISOBase *ioStream,TheClock::duration pingStatusReportPeriod = std::chrono::seconds(0),ThroughputReportFunctionType reportFunction=&noReportingFunction);

    ~MasterThroughput () override = default;

    void operator()() override;

    void handleReceive(boost::system::error_code const & ec,std::size_t bytesAvailable);

    void getResults ();
    void makeReport () override;
    void receiveResults (const boost::system::error_code &ec, std::size_t bytesAvailable);

    static void noReportingFunction (bool b,uint32_t r,uint32_t u);

protected:
    std::array<uint8_t, 2*sizeof(uint32_t)> _recvBuf;
    volatile bool _gotResults;
    volatile bool _resultsReady;
    uint32_t _totalReceived;
    uint32_t _totalUsec;
};

class MasterThroughputFactory : public CommunicatingRunnableFactory<MasterThroughput> {
public:
    explicit MasterThroughputFactory (AISOBase *ioStream);

    static const TheClock::duration &getPingStatusPrintPeriod ();

    static void setPingStatusPrintPeriod (const TheClock::duration &pingStatusPrintPeriod);
    static void setReportingFunction (const ThroughputReportFunctionType &reportingFunction);

    MasterThroughput* operator()() override;
protected:
    static TheClock::duration _pingStatusPrintPeriod;
    static ThroughputReportFunctionType _reportingFunction;
};

#endif //MODEMTESTER_MASTERTHROUGHPUT_H
