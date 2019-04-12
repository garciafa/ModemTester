//
// Created by garciafa on 15/03/19.
//

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <csignal>
#include <boost/program_options.hpp>
#include <Ivy/Ivycpp.h>
#include <fstream>
#include "../logging.h"

using myclock = std::chrono::high_resolution_clock ;

Ivy *bus = nullptr;

myclock::time_point begin;

void sighandler(int signal)
{
    BOOST_LOG_TRIVIAL(warning) << "Received signal " << signal << " stopping." << std::endl;
    if (bus!= nullptr)
    {
        bus->stop();
    }
}

class IvyLogger : public IvyMessageCallback  {
public:
    explicit IvyLogger (std::string &filename)
    {
        _os = new std::fstream(filename,std::ios::out);
    }

    IvyLogger (): _os(&std::cout){}

    void OnMessage (IvyApplication *app, int argc, const char **argv ) override
    {
        std::chrono::duration<double> date = myclock::now()-begin;
        (*_os) << std::fixed << std::setprecision( 3 ) <<  date.count() << " ";
        for (int i=0; i<argc;++i)
        {
            (*_os) << argv[i] << " ";
        }
        (*_os) << std::endl;
    }
    ~IvyLogger() override
    {
        _os->flush();

        if (_os != &std::cout)
        {
            delete _os;
        }
    }
protected:
    std::ostream *_os;
};

int main(int argc, char** argv)
{
    unsigned int logLevel;
    std::string busAddr;
    IvyLogger *cb = nullptr;
    begin = myclock::now();
    time_t begin_time_t = myclock::to_time_t(begin);
    constexpr size_t size = 256;
    char buf[size];
    strftime(buf,size,"%0y_%0m_%0d__%0H_%0M_%0S.log",std::localtime(&begin_time_t));
    std::string filename{buf};

    boost::program_options::options_description desc("Options ");
    desc.add_options()
        ("help,h", "help message")
        ("output,o", boost::program_options::value<std::string>(&filename)->default_value(filename),"Filename to log to (- for stdout)")
        ("bus,b", boost::program_options::value<std::string>(&busAddr)->default_value("127.255.255.255:2010"),"Ivy bus to use")
        ("log_level,l",boost::program_options::value<unsigned int> (&logLevel)->default_value(3),"Log level from 0 (fatal errors) to 5 (traces)");

    try
    {
        boost::program_options::options_description cmdLine;
        cmdLine.add(desc);

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(cmdLine).run(), vm);
        boost::program_options::notify (vm);

        if (vm.count ("help"))
        {
            std::cout << argv[0] << " [options] <serial path>" << std::endl;
            std::cout << desc << std::endl;
            exit(EXIT_SUCCESS);
        }
        if (logLevel < 0 || logLevel > 5)
        {
            std::cerr << "Error : Logging level can only range from 0 to 5." << std::endl;
            exit(EXIT_FAILURE);
        }
        if (filename=="-")
        {
            cb = new IvyLogger();
            BOOST_LOG_TRIVIAL(info) << "Output to stdout" << std::endl;
        }
        else
        {
            cb = new IvyLogger(filename);
            BOOST_LOG_TRIVIAL(info) << "Output to " << filename << std::endl;
        }
    }
    catch (boost::program_options::unknown_option &err)
    {
        std::cerr << err.what () << std::endl;
        std::cout << argv[0] << " [options] <path>" << std::endl;
        std::cout << desc << std::endl;
        exit(EXIT_FAILURE);
    }
    setLogLevel(static_cast<logging::trivial::severity_level>(logging::trivial::fatal - logLevel));

    signal(SIGTERM,sighandler);
    signal(SIGKILL,sighandler);
    signal(SIGINT,sighandler);

    bus = new Ivy("IvyLogger","Ready",new IvyApplicationNullCallback(),0);

    bus->BindMsg("^(ground AVAIL_REPORT .*)$",cb);
    bus->BindMsg("^(ground THROUGHPUT_REPORT .*)$",cb);
    bus->BindMsg("^(ground FLIGHT_PARAM .*)$",cb);
    bus->BindMsg("^(.* GPS .*)$",cb);

    bus->start(busAddr.c_str());

    bus->ivyMainLoop();

    delete cb;
    cb = nullptr;
    delete bus;
    bus = nullptr;

    return EXIT_SUCCESS;
}