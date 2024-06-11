#ifndef CONSENSUS_EXTENT_H
#define CONSENSUS_EXTENT_H

#include <vector>
#include <string_view>
#include <fstream>
#include "Extent.h"
#include "BitVector.h"
#include "Serializer.h"


struct ExtentBit{
    bool value;
    float prob;
};

class Extent {
    friend class File;
    std::uint32_t N_BLOCKS = 0;
    std::uint32_t BLOCK_SIZE = 0;

    std::uint64_t _blockIdx = 0;
    std::uint64_t _bitIdx = 0;
    std::vector<BitVector> blocks;
    std::vector<std::vector<float>> probability;


    bool GetAt(std::uint64_t blockIdx, std::uint64_t bitIdx);
    void SetAt(std::uint64_t blockIdx, std::uint64_t bitIdx, bool value);
    bool IncPtr();

public:
    explicit Extent(std::uint32_t N_BLOCKS, std::uint32_t BLOCK_SIZE,/*std::shared_ptr<Serializer> serializer,size_t NUM_BLOCKS, size_t SIZE_LDPC_BLOCK,
           size_t INDEX_POSITION, size_t INDEX_LEN, size_t BITVALUE_SIZE, size_t RANDOM_DIM,*/ size_t RANDOM_SEED); // TODO: Remove RANDOM_SEED

    void ResetPtr();
    bool GetNextBit(ExtentBit& nextBit);
    bool SetNextBit(ExtentBit bit);

    void RandomizeBlocks(std::uint64_t seed);
    std::vector<std::vector<int64_t>> SerializeData(Serializer& s);

    void StreamOnFile(std::string_view filePath);

    void WriteOnFile(std::ofstream& fileStream, std::uint64_t szBinaryFile);

    void Fill(const std::vector<byte_t>& srcData);

    const std::vector<BitVector>& GetBlocks();

    //friend std::istream& operator>>(std::istream &is, Extent& vector);
    void SetBlockFromString(std::uint64_t blockIdx, std::string_view block);

    void ZeroingContent();
};

#endif //CONSENSUS_EXTENT_H
