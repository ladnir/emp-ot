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
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /EHsc -D_CRT_SECURE_NO_WARNINGS /Z7 /DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Od")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF" CACHE STRING "" FORCE)
else()
	set(CMAKE_C_FLAGS "-pthread -Wall -march=native -O3 -maes")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14")




set(LIBRARIES "${EMP-TOOL_LIBRARIES};${relic_LIBRARIES};${cryptoTools_LIBRARIES};${GMP_LIBRARIES}")
#set(DEBUG_LIBRARIES "${EMP-TOOL_DEBUG_LIBRARIES};${relic_DEBUG_LIBRARIES};${cryptoTools_DEBUG_LIBRARIES};${GMP_DEBUG_LIBRARIES}")


add_executable(shot    test/shot.cpp)
add_executable(mot     test/mot.cpp)
add_executable(iter_ot test/iter_ot.cpp)
target_link_libraries(shot    ${LIBRARIES}) 
target_link_libraries(mot     ${LIBRARIES})
target_link_libraries(iter_ot ${LIBRARIES})




install(FILES emp-ot-config.cmake DESTINATION cmake/)

file(GLOB  headers 
	"ot/*.h"
)
install(FILES ${headers}  DESTINATION include/${NAME})
install(FILES emp-ot.h  DESTINATION include)
