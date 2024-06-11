cmake_minimum_required(VERSION 3.18)
project(RefactoringOligoArchive)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")

FIND_PACKAGE(Boost 1.63.0 COMPONENTS program_options filesystem REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

find_package(TBB)


add_executable(encoder main_encoder.cpp
        ../../src/AlignmentTools/edlib/src/edlib.cpp
        ../../src/AlignmentTools/Aligner.cpp
        ../../src/Common/Logger.cpp
        ../../src/Codec/Codec.cpp
        ../../src/Codec/Encoder.cpp
        ../../src/DataStructure/Motif.cpp
        ../../src/DataStructure/BitVector.cpp
        ../../src/DataStructure/Cluster.cpp
        ../../src/DataStructure/Extent.cpp
        ../../src/DataStructure/Serializer.cpp
        ../../src/Common/Utils.cpp
        ../../src/Common/MD5.cpp
        ../../src/DataStructure/File.cpp ../../src/DataStructure/File.h
        Settings/CodecConfiguration.cpp
        Settings/FileConfiguration.cpp
        Settings/Constants.h Settings/Generic.cpp)

target_link_libraries( encoder TBB::tbb ${Boost_LIBRARIES} stdc++fs )

add_executable(decoder main_decoder.cpp
        ../../src/AlignmentTools/edlib/src/edlib.cpp
        ../../src/AlignmentTools/Aligner.cpp
        ../../src/Common/Logger.cpp
        ../../src/Codec/Codec.cpp
        ../../src/Codec/Decoder.cpp
        ../../src/Codec/Encoder.cpp
        ../../src/DataStructure/Motif.cpp
        ../../src/DataStructure/BitVector.cpp
        ../../src/DataStructure/Cluster.cpp
        ../../src/DataStructure/Extent.cpp
        ../../src/DataStructure/Serializer.cpp
        ../../src/Common/Utils.cpp
        ../../src/Common/MD5.cpp
        ../../src/DataStructure/File.cpp ../../src/DataStructure/File.h
        Settings/CodecConfiguration.cpp
        Settings/FileConfiguration.cpp
        Settings/Constants.h
        Settings/Generic.cpp Settings/Generic.h)

target_link_libraries( decoder TBB::tbb ${Boost_LIBRARIES} stdc++fs)