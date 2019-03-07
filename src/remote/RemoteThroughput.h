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
// Created by garciafa on 05/03/19.
//

#ifndef MODEMTESTER_REMOTETHROUGHPUT_H
#define MODEMTESTER_REMOTETHROUGHPUT_H

#include "../Runnable.h"
#include "../CommunicatingRunnableFactory.h"
#include "../TimeLogger.h"

class RemoteThroughput : public Runnable {
public:
    void operator() () override;

    ~RemoteThroughput() override = default;

    void handleReceive (boost::system::error_code const &ec, std::size_t bytesAvailable);

    explicit RemoteThroughput (AISOBase *ioStream);

    void sendResults();

protected:
    AISOBase *_ioStream;
    std::array<uint8_t, 255> _recvBuf; // FIXME the size should be settable
    std::map <TheClock::time_point,size_t> _recvedBytes;
};

using RemoteThroughputFactory = CommunicatingRunnableFactory<RemoteThroughput>;

#endif //MODEMTESTER_REMOTETHROUGHPUT_H
