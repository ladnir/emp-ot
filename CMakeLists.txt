cmake_minimum_required (VERSION 2.8.11)
project (emp-ot)
set(NAME "emp-ot")

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH} /usr/local")
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ot)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} cmake/)
FIND_PACKAGE(emp-tool)
#INCLUDE_DIRECTORIES(${2PCTOOL_INCLUDE_DIRS})
#message(${2PCTOOL_INCLUDE_DIRS})

set (CMAKE_C_FLAGS "-pthread -Wall -march=native -O3 -maes")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x")

add_executable(test_ot test/test_ot.cpp)
target_link_libraries(test_ot  relic  ${OPENSSL_LIBRARIES} gmp ${EMP-TOOL_LIBRARIES}) 

install(FILES emp-ot-config.cmake DESTINATION cmake/)

file(GLOB  headers 
"include/*.h"
"emp-ot.h"
)
install(FILES ${headers}  DESTINATION include/${NAME})