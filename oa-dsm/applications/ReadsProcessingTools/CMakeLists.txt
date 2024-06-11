cmake_minimum_required(VERSION 3.18)
project(ReadsProcessing)

set(CMAKE_CXX_STANDARD 20)

FIND_PACKAGE(Boost 1.63.0 COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
FIND_PACKAGE(TBB REQUIRED tbb)

add_executable(GroupReads main_reads_grouping.cpp Utils.cpp ../../src/AlignmentTools/Aligner.cpp ../../src/AlignmentTools/edlib/src/edlib.cpp)
target_link_libraries(GroupReads ${Boost_LIBRARIES} TBB::tbb)

add_executable(RemovePrimers main_primers_removal.cpp Utils.cpp ../../src/AlignmentTools/Aligner.cpp ../../src/AlignmentTools/edlib/src/edlib.cpp)
target_link_libraries(RemovePrimers ${Boost_LIBRARIES} TBB::tbb)

add_executable(StandaloneConsensus main_consensus.cpp Utils.cpp ../../src/AlignmentTools/Aligner.cpp ../../src/AlignmentTools/edlib/src/edlib.cpp)
target_link_libraries(StandaloneConsensus ${Boost_LIBRARIES} TBB::tbb)
