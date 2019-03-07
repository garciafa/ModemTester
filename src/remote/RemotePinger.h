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

#ifndef MODEMTESTER_REMOTEPINGER_H
#define MODEMTESTER_REMOTEPINGER_H

#include "../Runnable.h"
#include "../CommunicatingRunnableFactory.h"

class RemotePinger : public Runnable {
public:
    void operator() () override;

    ~RemotePinger() override = default;

    void handleReceive (boost::system::error_code const &ec, std::size_t bytesAvailable);

    RemotePinger (AISOBase *ioStream);
protected:
    AISOBase *_ioStream;
    std::array<uint8_t, 2> _recvBuf;
};

using RemotePingerFactory = CommunicatingRunnableFactory<RemotePinger>;

#endif //MODEMTESTER_REMOTEPINGER_H
