//
// Created by garciafa on 13/03/19.
//

#include "logging.h"

void setLogLevel(const logging::trivial::severity_level &level)
{
    logging::core::get()->set_filter
        (
            logging::trivial::severity >= level
        );
}
