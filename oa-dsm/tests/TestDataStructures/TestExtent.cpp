#include <gtest/gtest.h>
#include "../../DataStructure/Extent.h"

const std::uint32_t N_BLOCKS = 19;
const std::uint32_t BLOCK_SIZE = 256000;


TEST(Extent, ExtentBasics) {

    size_t szFile = 60000; // Bytes
    std::vector<byte_t> srcData(szFile, 0x00);

    Extent extent(N_BLOCKS, BLOCK_SIZE, 0);

    extent.Fill(srcData);

    ExtentBit bit{};
    std::uint64_t bitCounter = 0;
    for (size_t idx = 0; idx < 60000*8; idx++, bitCounter++) {
        extent.GetNextBit(bit);
        EXPECT_EQ(bit.value, 0);
    }
}

TEST(Extent, GetSetNext) {
    Extent extent(N_BLOCKS, BLOCK_SIZE, 0);

    ExtentBit bit{};
    std::uint64_t bitCounter = 0;
    while (extent.GetNextBit(bit)) {
        bitCounter++;
    }
    // Checking the index
    std::uint64_t maxIdx = N_BLOCKS * BLOCK_SIZE - 1;
    EXPECT_EQ(bitCounter, maxIdx);
}

TEST(Extent, Randomization) {

    size_t szFile = 60000; // Bytes
    std::vector<byte_t> srcData(szFile, 0xFF);

    Extent extent(N_BLOCKS, BLOCK_SIZE, 0);

    extent.Fill(srcData);
    extent.RandomizeBlocks(152463);
    extent.RandomizeBlocks(152463);

    ExtentBit bit{};
    for (size_t idx = 0; idx < 60000*8; idx++) {
        extent.GetNextBit(bit);
        EXPECT_EQ(bit.value, 1);
    }
}



TEST(Extent, WriteOut) {
    size_t szFile = 1; // Bytes
    std::vector<byte_t> srcData1(szFile, 0xFA);
    {
        std::ofstream file("./tmp_file");
        Extent extent(N_BLOCKS, BLOCK_SIZE, 0);
        extent.Fill(srcData1);
        extent.WriteOnFile(file, szFile);
    }
    std::vector<byte_t> srcData2(szFile, 0);
    {
        std::ifstream srcFile("./tmp_file", std::ios::binary);
        srcFile.read(reinterpret_cast<char*>(srcData2.data()), szFile);
    }
    for (size_t idx = 0; idx < szFile; idx++) {
        EXPECT_EQ(srcData1[idx], srcData2[idx]);
    }
    system("rm ./tmp_file");
}

TEST(Extent, WriteOutLargeFile) {
    size_t szFile = 1<<20; // Bytes
    std::vector<byte_t> srcData1(szFile, 0);

    for (size_t idx = 0; idx < szFile; idx++) {
        srcData1[idx] = std::rand() % 256;
    }

    {
        std::ofstream file("./tmp_file");
        Extent extent(N_BLOCKS, BLOCK_SIZE, 0);
        extent.Fill(srcData1);
        EXPECT_THROW(extent.WriteOnFile(file, szFile), invalid_size );
    }

    szFile = 1<<18; // Bytes
    srcData1.resize(szFile, 0);

    for (size_t idx = 0; idx < szFile; idx++) {
        srcData1[idx] = std::rand() % 256;
    }

    {
        std::ofstream file("./tmp_file");
        Extent extent(N_BLOCKS, BLOCK_SIZE, 0);
        extent.Fill(srcData1);
        EXPECT_NO_THROW( extent.WriteOnFile(file, szFile) );
    }

    std::vector<byte_t> srcData2(szFile, 0);
    {
        std::ifstream srcFile("./tmp_file", std::ios::binary);
        srcFile.read(reinterpret_cast<char*>(srcData2.data()), szFile);
    }
    for (size_t idx = 0; idx < szFile; idx++) {
        EXPECT_EQ(srcData1[idx], srcData2[idx]);
    }
    system("rm ./tmp_file");
}