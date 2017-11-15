cmake_minimum_required(VERSION 2.8)
project(jsgf_parser)

set(TOPDIR .)

include_directories(
        ${TOPDIR}/include
        ${TOPDIR}/src
)

#MESSAGE(STSTUS "###################################")
#MESSAGE(STATUS "CMAKE_SYSTEM： ${CMAKE_SYSTEM}")
#MESSAGE(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
#MESSAGE(STSTUS "###################################")

if (APPLE)
    MESSAGE(STSTUS " - os is macos")
else()
    MESSAGE(STSTUS " - os is linux")
endif()

set(SOURCES
        ${TOPDIR}/src/util/splitter.cpp
        ${TOPDIR}/src/util/timer.cpp
        ${TOPDIR}/src/util/printer.cpp
        ${TOPDIR}/src/dict/dict_matcher.cpp
        ${TOPDIR}/src/dict/regex_matcher.cpp
        ${TOPDIR}/src/jsgf/case.c
        ${TOPDIR}/src/jsgf/ckd_alloc.c
        ${TOPDIR}/src/jsgf/strfuncs.c
        ${TOPDIR}/src/jsgf/dtoa.c
        ${TOPDIR}/src/jsgf/glist.c
        ${TOPDIR}/src/jsgf/hash_table.c
        ${TOPDIR}/src/jsgf/jsgf_parser.c
        ${TOPDIR}/src/jsgf/jsgf_scanner.c
        ${TOPDIR}/src/jsgf/jsgf.c
        ${TOPDIR}/src/wfst/rule.cpp
        ${TOPDIR}/src/wfst/parser.cpp
        )


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Wextra -Wno-unused-parameter")
add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)

if (APPLE)
    add_definitions(-D_DARWIN_USE_64_BIT_INODE=1 -D_DARWIN_UNLIMITED_SELECT=1)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_BUILE_TYPE DEBUG)
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
else()
    add_definitions(-Wstrict-aliasing)
    add_definitions(-std=c++11)
    add_definitions(-fPIC)
    add_definitions(-g)
    add_definitions(-O2)
endif()

if ((${CMAKE_SYSTEM_NAME} MATCHES "Linux") OR (${CMAKE_SYSTEM_NAME} MATCHES "Android"))
    add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE=200112)
endif()

find_path(SIMPLELOG_HEADER NAMES simple_log.h)
find_path(CPPJSON_HEADER NAMES json/json.h)
include_directories(${SIMPLELOG_HEADER} ${CPPJSON_HEADER})

add_library(jsgfparser SHARED ${SOURCES})

find_library(SIMPLELOG_LIB NAMES simplelog)
find_library(RE2_LIB NAMES re2)
find_library(CPPJSON_LIB NAMES cppjson)
find_library(M_LIB NAMES m)
find_package(Threads)
target_link_libraries(jsgfparser ${CMAKE_THREAD_LIBS_INIT} ${M_LIB} ${SIMPLELOG_LIB} ${RE2_LIB} ${CPPJSON_LIB})


#install directory
# cmake -DCMAKE_INSTALL_PREFIX=/usr
install(TARGETS jsgfparser
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

install(
        DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*")


#######################Test Unit########################
set(SplitterTest_SOURCES
        ${TOPDIR}/test/SplitterTest.cpp
        )
add_executable(SplitterTest ${SplitterTest_SOURCES})
target_link_libraries(SplitterTest jsgfparser)

set(DictMatcherTest_SOURCES
        ${TOPDIR}/test/DictMatcherTest.cpp
        )
add_executable(DictMatcherTest ${DictMatcherTest_SOURCES})
target_link_libraries(DictMatcherTest jsgfparser)

set(JsgfGramTest_SOURCES
        ${TOPDIR}/test/JsgfGramTest.cpp
        )
add_executable(JsgfGramTest ${JsgfGramTest_SOURCES})
target_link_libraries(JsgfGramTest jsgfparser)

set(WfstParserTest_SOURCES
        ${TOPDIR}/test/WfstParserTest.cpp
        )
add_executable(WfstParserTest ${WfstParserTest_SOURCES})
target_link_libraries(WfstParserTest jsgfparser)

set(PrinterTest_SOURCES
        ${TOPDIR}/test/PrinterTest.cpp
        )
add_executable(PrinterTest ${PrinterTest_SOURCES})
target_link_libraries(PrinterTest jsgfparser)

set(BatchTest_SOURCES
        ${TOPDIR}/test/BatchTest.cpp
        )
add_executable(BatchTest ${BatchTest_SOURCES})
target_link_libraries(BatchTest jsgfparser)

