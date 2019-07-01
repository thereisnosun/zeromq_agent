## i have cmake 3.5
cmake_minimum_required(VERSION 3.5)
project(test2)

## use this to globally use C++11 with in our project
set(CMAKE_CXX_STANDARD 17)

## load in pkg-config support
find_package(PkgConfig)

# ZeroMQ
pkg_check_modules(ZMQ REQUIRED libzmq)
include_directories(${ZMQ_INCLUDE_DIRS})
link_directories(${ZMQ_LIBRARY_DIRS})
add_definitions(${ZMQ_CFLAGS_OTHER})
set(LINKLIBS ${LINKLIBS} ${ZMQ_LIBRARIES})

add_executable(wuserver wuserver.c consts.h)
target_include_directories(wuserver PUBLIC ${ZMQ_INCLUDE_DIRS})
target_link_libraries(wuserver PUBLIC ${LINKLIBS})

add_executable(wuclient wuclient.c consts.h agent_api.h)
target_include_directories(wuclient PUBLIC ${ZMQ_INCLUDE_DIRS})
target_link_libraries(wuclient PUBLIC ${LINKLIBS})

add_executable(zeromq_agent agent_api.h broker_api.h zeromq_agent.cpp)
target_include_directories(zeromq_agent PUBLIC ${ZMQ_INCLUDE_DIRS})
target_link_libraries(zeromq_agent PUBLIC ${LINKLIBS})

install(TARGETS wuserver DESTINATION /usr/bin COMPONENT binaries)
install(TARGETS wuclient DESTINATION /usr/bin COMPONENT binaries)
install(TARGETS zeromq_agent DESTINATION /usr/bin COMPONENT binaries)
