cmake_minimum_required(VERSION 3.13)
project(ModemTester)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-Wall")

find_package (Boost REQUIRED program_options log log_setup)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif (Boost_FOUND)

set (MASTER_SOURCES
        src/master/MasterFSM.cpp
        src/master/MasterFSM.h
        src/master/MasterPinger.cpp
        src/master/MasterPinger.h
        src/master/MasterThroughput.cpp
        src/master/MasterThroughput.h)

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
        src/TimeLogger.cpp
        src/TimeLogger.h
        src/Events.h
        src/BaseFSM.h
        src/Events.cpp
        src/CommunicatingRunnableFactory.h
        src/EventsScript.h
        src/ReportingActivity.h src/RunnablePool.cpp src/RunnablePool.h)

add_executable(${PROJECT_NAME} src/main.cpp ${COMMON_SOURCES} ${MASTER_SOURCES} ${REMOTE_SOURCES})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} stdc++fs pthread)

add_executable(testMasterFSM src/tests/testMasterFSM.cpp ${COMMON_SOURCES} ${MASTER_SOURCES})
target_link_libraries(testMasterFSM ${Boost_LIBRARIES} stdc++fs pthread)

add_executable(testRemoteFSM src/tests/testRemoteFSM.cpp ${COMMON_SOURCES} ${REMOTE_SOURCES})
target_link_libraries(testRemoteFSM ${Boost_LIBRARIES} stdc++fs pthread)

#add_executable(testAsyncIO src/AISO.h src/tests/testAsyncIO.cpp src/TimeLogger.cpp src/TimeLogger.h)
#target_link_libraries(testAsyncIO ${Boost_LIBRARIES} stdc++fs pthread)

#add_executable(testTimerCancel src/TimeLogger.cpp src/TimeLogger.h src/tests/testTimerCancel.cpp)
#target_link_libraries(testTimerCancel ${Boost_LIBRARIES} stdc++fs pthread)