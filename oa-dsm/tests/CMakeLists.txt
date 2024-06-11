cmake_minimum_required(VERSION 3.18)
project(RefactoringOligoArchive)

set(CMAKE_CXX_STANDARD 20)

FIND_PACKAGE(Boost 1.63.0 COMPONENTS program_options log log_setup filesystem REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

find_package(TBB)


include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        TestMotif
        TestDataStructures/TestMotifs.cpp ../src/DataStructure/Motif.cpp
)
target_link_libraries(
        TestMotif
        GTest::gtest_main
)


add_executable(
        TestOligo
        TestDataStructures/TestOligo.cpp ../src/DataStructure/Motif.cpp
)
target_link_libraries(
        TestOligo
        GTest::gtest_main
)

add_executable(
        TestCodec
        TestCodec/TestCodec.cpp ../src/Codec/Codec.cpp ../src/Common/Logger.cpp ../src/DataStructure/Motif.cpp
        ../src/Codec/CodecExceptions.h
        ../src/Common/Logger.cpp
)
target_link_libraries(
        TestCodec
        GTest::gtest_main pthread TBB::tbb ${Boost_LIBRARIES} stdc++fs )


add_executable(
        TestBitVector
        TestDataStructures/TestBitVector.cpp ../src/DataStructure/BitVector.cpp
)
target_link_libraries(
        TestBitVector
        GTest::gtest_main TBB::tbb ${Boost_LIBRARIES}
)

add_executable(
        TestDecoder
        TestCodec/TestDecoder.cpp
        ../src/DataStructure/BitVector.cpp
        ../src/Codec/Decoder.cpp ../src/Codec/Codec.cpp
        ../src/DataStructure/Motif.cpp
        ../src/Common/Logger.cpp
)
target_link_libraries(
        TestDecoder
        GTest::gtest_main pthread TBB::tbb ${Boost_LIBRARIES} stdc++fs )

add_executable(
        TestEncoder
        TestCodec/TestEncoder.cpp
        ../src/DataStructure/BitVector.cpp
        ../src/Codec/Encoder.cpp ../src/Codec/Codec.cpp
        ../src/DataStructure/Motif.cpp
        ../src/Common/Logger.cpp
)
target_link_libraries(
        TestEncoder
        GTest::gtest_main pthread TBB::tbb ${Boost_LIBRARIES} stdc++fs )

add_executable(
        TestExtent
        TestDataStructures/TestExtent.cpp
        ../src/DataStructure/BitVector.cpp
        ../src/DataStructure/Motif.cpp
        ../src/DataStructure/Extent.cpp
)
target_link_libraries(
        TestExtent
        GTest::gtest_main TBB::tbb ${Boost_LIBRARIES}
)

add_executable(
        TestSerializer
        TestDataStructures/TestSerializer.cpp
        ../src/DataStructure/BitVector.cpp
        ../src/DataStructure/Motif.cpp
        ../src/DataStructure/Serializer.cpp
        ../src/DataStructure/Extent.cpp
)
target_link_libraries(
        TestSerializer
        GTest::gtest_main TBB::tbb ${Boost_LIBRARIES}
)

add_executable(
        TestEncodingDecoding
        TestUseCase/TestEncodingDecoding.cpp
        ../src/AlignmentTools/edlib/src/edlib.cpp
        ../src/AlignmentTools/Aligner.cpp
        ../src/Common/Logger.cpp
        ../src/Codec/Codec.cpp
        ../src/Codec/Decoder.cpp
        ../src/Codec/Encoder.cpp
        ../src/Common/Utils.cpp
        ../src/Common/MD5.cpp
        ../src/DataStructure/Motif.cpp
        ../src/DataStructure/BitVector.cpp
        ../src/DataStructure/Cluster.cpp
        ../src/DataStructure/Extent.cpp
        ../src/DataStructure/Serializer.cpp
        ../applications/XCoder/Settings/CodecConfiguration.cpp
        ../applications/XCoder/Settings/FileConfiguration.cpp
        ../applications/XCoder/Settings/Constants.h
)
target_link_libraries(
        TestEncodingDecoding
        GTest::gtest_main TBB::tbb ${Boost_LIBRARIES} stdc++fs
)

add_executable(
        TestClusters
        TestDataStructures/TestCluster.cpp
        ../src/AlignmentTools/Aligner.cpp
        ../src/AlignmentTools/edlib/src/edlib.cpp
        ../src/DataStructure/Cluster.cpp
)
target_link_libraries(
        TestClusters
        GTest::gtest_main TBB::tbb ${Boost_LIBRARIES}
)

add_executable(
        TestLogger
        TestCommon/TestLogger.cpp
        ../src/Common/Logger.cpp
)
target_link_libraries(
        TestLogger
        GTest::gtest_main TBB::tbb ${Boost_LIBRARIES}
)

add_executable(
        TestAlignment
        TestAlignment/TestAlignment.cpp
        ../src/AlignmentTools/Aligner.cpp
        ../src/AlignmentTools/edlib/src/edlib.cpp
        ../src/Common/Logger.cpp
)
target_link_libraries(
        TestAlignment
        GTest::gtest_main TBB::tbb ${Boost_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(TestMotif)
gtest_discover_tests(TestOligo)
gtest_discover_tests(TestBitVector)
gtest_discover_tests(TestExtent)
gtest_discover_tests(TestLogger)
gtest_discover_tests(TestAlignment)
gtest_discover_tests(TestClusters)
gtest_discover_tests(TestSerializer)
gtest_discover_tests(TestCodec)
gtest_discover_tests(TestEncoder)
gtest_discover_tests(TestDecoder)
