//
// Created by garciafa on 28/02/19.
//

#include <cstdlib>
#include <iostream>
#include <functional>
#include "../master/MasterFSM.h"
#include "../TimeLogger.h"
#include "../EventsScript.h"

FSM_INITIAL_STATE(MasterFSM,Idle);

using boost::asio::io_service;

using boost::asio::ip::tcp;

boost::asio::basic_waitable_timer<TheClock> *printTimer;

constexpr auto printStatusPeriod = std::chrono::seconds(1);

void printState(boost::system::error_code const & ec)
{
    printTimer->expires_from_now(printStatusPeriod);
    std::cout << TimeLogger::now().count() << " # Current state : " << *MasterFSM::current_state_ptr << std::endl;
    printTimer->async_wait(&printState);
}

void AvailabilityReportingFunction(unsigned long sent,unsigned long lost,unsigned long received,double avRtt)
{
    std::cout << "----------------------------------------------" << std::endl;
    std::cout << TimeLogger::now().count() << " # AvailabilityMode report" << std::endl;
    std::cout << "\tSent " << sent << " bytes\n";
    std::cout << "\tReceived " << received << " bytes\n";
    std::cout << "\tLost " << lost << " bytes\n";
    std::cout << "\tAvRTT " << avRtt*1000 << " ms\n";
    std::cout << "----------------------------------------------" << std::endl;
}

void ThroughputReportingFunction(bool valid,uint32_t received, uint32_t usecs)
{
    double sec = usecs / 1000000.0;
    std::cout << "----------------------------------------------" << std::endl;
    std::cout << TimeLogger::now().count() << " # ThroughputMode report" << std::endl;
    std::cout << "\tReceived " << received << " Bytes in " << sec << " s" << std::endl;
    std::cout << "\tThroughput " << received / sec << " Bytes/s" << std::endl;
    std::cout << "\tThroughput " << 8 * received / sec << " b/s" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;

}

using std::chrono::seconds;

int main()
{
    try
    {
        io_service io;

        MasterPingerFactory::setPingStatusPrintPeriod(seconds(0));
        MasterPingerFactory::setReportingFunction(AvailabilityReportingFunction);
        MasterThroughputFactory::setPingStatusPrintPeriod(seconds(0));
        MasterThroughputFactory::setReportingFunction(ThroughputReportingFunction);

        EventsScript<MasterFSM> eventsScript(io,true);
        auto deadline = seconds(0);
        eventsScript[deadline+=seconds(1)] = AvailabilityModeCommand_id;
        eventsScript[deadline+=seconds(20)] = EndModeCommand_id;
        eventsScript[deadline+=seconds(1)] = ThroughputModeCommand_id;
        eventsScript[deadline+=seconds(5)] = EndModeCommand_id;
        eventsScript[deadline+=seconds(1)] = EndTest_id;

        tcp::socket socket(io);
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 2020));
        boost::system::error_code ec;
        acceptor.accept(socket, ec);

        AISOBase *ptr = AISOBase::CreateFromStream(socket);

        TimeLogger::reset();
        MasterFSM::setIoStream(ptr);
        MasterFSM::start();
        eventsScript.startScript();

        unsigned long res=1;
        printTimer = new boost::asio::basic_waitable_timer<TheClock>(ptr->get_io_service(), printStatusPeriod);
        printTimer->async_wait(&printState);

        while (res)
            res=io.run_one();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error in main loop : " << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}