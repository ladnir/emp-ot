cmake_minimum_required (VERSION 2.8.11)
project (emp-ot)
set(NAME "emp-ot")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} cmake/ ${CMAKE_SOURCE_DIR}/../emp-tool)
FIND_PACKAGE(emp-tool    REQUIRED)
FIND_PACKAGE(cryptoTools REQUIRED)
FIND_PACKAGE(relic       REQUIRED)
FIND_PACKAGE(gmp         REQUIRED)


include_directories(${CMAKE_SOURCE_DIR}/..)
include_directories(${EMP-TOOL_INCLUDE_DIRS} ${cryptoTools_INCLUDE_DIRS} ${relic_INCLUDE_DIRS} ${GMP_INCLUDE_DIRS})
LINK_DIRECTORIES(${cryptoTools_LINK_DIRS})


if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /EHsc -D_CRT_SECURE_NO_WARNINGS")
	set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
else()
	set(CMAKE_C_FLAGS "-pthread -Wall -march=native -O3 -maes")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14")



set (LIBS "${EMP-TOOL_LIBRARIES};${cryptoTools_LIBRARIES};${relic_LIBRARIES};${GMP_LIBRARIES}")
add_executable(shot    test/shot.cpp)
add_executable(mot     test/mot.cpp)
add_executable(iter_ot test/iter_ot.cpp)
target_link_libraries(shot     ${LIBS}) 
target_link_libraries(mot      ${LIBS})
target_link_libraries(iter_ot  ${LIBS})




install(FILES emp-ot-config.cmake DESTINATION cmake/)

file(GLOB  headers 
"ot/*.h"
"emp-ot"
)
install(FILES ${headers}  DESTINATION include/${NAME})
