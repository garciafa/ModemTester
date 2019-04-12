//
// Created by garciafa on 13/03/19.
//

#ifndef MODEMTESTER_LOGGING_H
#define MODEMTESTER_LOGGING_H

#define BOOST_ALL_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace logging = boost::log;

void setLogLevel(const logging::trivial::severity_level &level);

#endif //MODEMTESTER_LOGGING_H
