cmake_minimum_required(VERSION 3.18)
project(ReadsSimulator)

set(CMAKE_CXX_STANDARD 20)

FIND_PACKAGE(Boost 1.63.0 COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
FIND_PACKAGE(TBB REQUIRED tbb)

add_executable(ReadsSimulator main_reads_simulator.cpp Utils.cpp)
target_link_libraries(ReadsSimulator ${Boost_LIBRARIES} TBB::tbb)

add_executable(ReadsSampler main_reads_sampler.cpp Utils.cpp )
target_link_libraries(ReadsSampler ${Boost_LIBRARIES} TBB::tbb)