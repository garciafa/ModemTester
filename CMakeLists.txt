#
# Copyright 2019 Fabien Garcia
# This file is part of ModemTester
#
# ModemTester is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ModemTester is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ModemTester.  If not, see <https://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.13)
project(ModemTester)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-Wall")

find_package (Boost REQUIRED program_options log log_setup)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif (Boost_FOUND)

include_directories(/usr/local/Ivy)
link_directories(/usr/local/lib64)

set (MASTER_SOURCES
        src/master/MasterFSM.cpp
        src/master/MasterFSM.h
        src/master/MasterPinger.cpp
        src/master/MasterPinger.h
        src/master/MasterThroughput.cpp
        src/master/MasterThroughput.h src/master/MasterController.cpp src/master/MasterController.h)

set (REMOTE_SOURCES
        src/remote/RemoteFSM.cpp
        src/remote/RemoteFSM.h
        src/remote/RemotePinger.cpp
        src/remote/RemotePinger.h
        src/remote/RemoteThroughput.cpp
        src/remote/RemoteThroughput.h)

set (COMMON_SOURCES
        src/Runnable.cpp
        src/Runnable.h
        src/AISO.h
        src/time_def.h
        src/Events.h
        src/BaseFSM.h
        src/Events.cpp
        src/CommunicatingRunnableFactory.h
        src/EventsScript.h
        src/ReportingActivity.h
        src/RunnablePool.cpp
        src/RunnablePool.h
        src/logging.cpp
        src/logging.h src/DispatchingRunnable.h)

add_executable(${PROJECT_NAME} src/main.cpp ${COMMON_SOURCES} ${MASTER_SOURCES} ${REMOTE_SOURCES})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} stdc++fs pthread Ivy ivy)

add_executable(testMasterFSM src/tests/testMasterFSM.cpp ${COMMON_SOURCES} ${MASTER_SOURCES})
target_link_libraries(testMasterFSM ${Boost_LIBRARIES} stdc++fs pthread Ivy ivy)

add_executable(testRemoteFSM src/tests/testRemoteFSM.cpp ${COMMON_SOURCES} ${REMOTE_SOURCES})
target_link_libraries(testRemoteFSM ${Boost_LIBRARIES} stdc++fs pthread)

add_executable(ivylogger src/ivylogger/ivylogger.cpp src/logging.cpp)
target_link_libraries(ivylogger ${Boost_LIBRARIES} stdc++fs pthread Ivy ivy)