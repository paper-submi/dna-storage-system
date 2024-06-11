cmake_minimum_required(VERSION 3.18)

project(OneConsensus)

set(CMAKE_CXX_STANDARD 17)

find_package( Boost 1.63.0 COMPONENTS program_options REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
find_package(TBB REQUIRED tbb)
#"$ENV{ONEAPI_ROOT}/dpl/2021.1-beta10/linux/include" )

if(NO_OUTPUT)
    add_definitions(-DDIS_PRINT_CENTROIDS)
    message("Centroids saving disabled")
else()
    remove_definitions(-DDIS_PRINT_CENTROIDS)
    message("Centroids saving enabled")
endif()
unset(NO_OUTPUT CACHE)


if(DEBUG)
    add_definitions(-DDEBUG)
    message("Printing clusters before applying cigar")
else()
    #remove_definitions(-DDIS_PRINT_CENTROIDS)
    #message("Centroids saving enabled")
endif()
unset(DEBUG CACHE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)