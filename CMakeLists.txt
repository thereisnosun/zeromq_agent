## i have cmake 3.5
cmake_minimum_required(VERSION 3.5)
project(test2)

## use this to globally use C++11 with in our project
set(CMAKE_CXX_STANDARD 17)

set(GCC_COVERAGE_COMPILE_FLAGS "-O1  -g3")
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

## load in pkg-config support
find_package(PkgConfig)

# ZeroMQ
pkg_check_modules(ZMQ REQUIRED libzmq)
include_directories(${ZMQ_INCLUDE_DIRS})
link_directories(${ZMQ_LIBRARY_DIRS})
add_definitions(${ZMQ_CFLAGS_OTHER})
set(LINKLIBS ${LINKLIBS} ${ZMQ_LIBRARIES})

#Boost
include_directories(${Boost_INCLUDE_DIR})
find_package(Boost 1.68 COMPONENTS system REQUIRED)
set(LINKLIBS ${LINKLIBS} ${Boost_LIBRARIES})


add_executable(wuserver wuserver.c consts.h)
target_include_directories(wuserver PUBLIC ${ZMQ_INCLUDE_DIRS})
target_link_libraries(wuserver PUBLIC ${LINKLIBS})

add_executable(wuclient wuclient.c consts.h)
target_include_directories(wuclient PUBLIC ${ZMQ_INCLUDE_DIRS})
target_link_libraries(wuclient PUBLIC ${LINKLIBS})

add_library(zeromq_agent SHARED
    broker_api.h
    client_api.h
    client_api.cpp
    general_api.h
    reactor_loop.h
    internal_utils.h
    server_api.h
    server_api.cpp
    broker_api.cpp
    end_points.h)

add_executable(zeromq_agent_test zeromq_agent.cpp
    broker_api.h
    client_api.h
    general_api.h
    server_api.h
    end_points.h)


target_include_directories(zeromq_agent PUBLIC ${ZMQ_INCLUDE_DIRS})
target_link_libraries(zeromq_agent PUBLIC ${LINKLIBS} pthread )

target_link_libraries(zeromq_agent_test PUBLIC zeromq_agent ${LINKLIBS} pthread )

install(TARGETS wuserver DESTINATION /usr/bin COMPONENT binaries)
install(TARGETS wuclient DESTINATION /usr/bin COMPONENT binaries)
install(TARGETS zeromq_agent_test DESTINATION /usr/bin COMPONENT binaries)
