//
// Created by garciafa on 01/03/19.
//

#ifndef MODEMTESTER_MASTERAVAILABILITY_H
#define MODEMTESTER_MASTERAVAILABILITY_H

#include "../Runnable.h"
#include "../AISO.h"
#include "../CommunicatingRunnableFactory.h"
#include "../TimeLogger.h"
#include "../ReportingActivity.h"

using AvailabilityReportFunctionType = std::function<void(unsigned long,unsigned long,unsigned long,double)>;

class MasterPinger : public ReportingActivity<AvailabilityReportFunctionType> {
public:
    explicit MasterPinger (AISOBase *ioStream,TheClock::duration pingStatusReportPeriod = std::chrono::seconds(0),std::function<void(unsigned long,unsigned long,unsigned long,double)> reportFunction=&noReportingFunction);

    ~MasterPinger() override=default;

    void operator()() override final;

    void handleReceive(boost::system::error_code const & ec,std::size_t bytesAvailable);
    unsigned long getSent () const;
    unsigned long getLost () const;
    unsigned long getReceived () const;
    double getAverageRtt () const;

    void printStats () const;

    void makeReport () override final;
    static void noReportingFunction(unsigned long sent,unsigned long lost,unsigned long received,double avRtt);

protected:
    std::array<uint8_t, 10> _recvBuf;
    unsigned long _sent;
    unsigned long _lost;
    unsigned long _received;
    double _rtt;
    std::map<uint8_t,TheClock::time_point> _sendTimes;
    boost::asio::basic_waitable_timer<TheClock> *_reportStatsTimer;
    TheClock::duration _pingStatusReportPeriod;
    AvailabilityReportFunctionType _reportingFunction;
};

class MasterPingerFactory : public CommunicatingRunnableFactory<MasterPinger> {
public:
    explicit MasterPingerFactory (AISOBase *ioStream);

    static const TheClock::duration &getPingStatusPrintPeriod ();

    static void setPingStatusPrintPeriod (const TheClock::duration &pingStatusPrintPeriod);
    static void setReportingFunction (const std::function<void (unsigned long, unsigned long, unsigned long, double)> &reportingFunction);

    MasterPinger* operator()() override;
protected:
    static TheClock::duration _pingStatusPrintPeriod;
    static std::function<void(unsigned long,unsigned long,unsigned long,double)> _reportingFunction;
};

#endif //MODEMTESTER_MASTERAVAILABILITY_H
