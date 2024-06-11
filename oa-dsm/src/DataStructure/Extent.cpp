#include <fstream>
#include <iostream>
#include <array>
#include <sstream>
#include <cassert>
#include "Extent.h"
#include "DataStructureExceptions.h"


Extent::Extent(std::uint32_t N_BLOCKS, std::uint32_t BLOCK_SIZE, size_t RANDOM_SEED) :
        N_BLOCKS(N_BLOCKS), BLOCK_SIZE(BLOCK_SIZE),
        _blockIdx(0),
        _bitIdx(0) {
    blocks.resize(N_BLOCKS, BitVector(BLOCK_SIZE));
    for (size_t bIdx = 0; bIdx < blocks.size(); bIdx++) {
        blocks[bIdx].RandomizeBits(RANDOM_SEED + bIdx);
    }
}

bool Extent::GetAt(std::uint64_t blockIdx, std::uint64_t bitIdx) {
    return blocks[blockIdx].GetBit(bitIdx);
}

void Extent::SetAt(std::uint64_t blockIdx, std::uint64_t bitIdx, bool value) {
    this->blocks[blockIdx].SetBit(bitIdx, value);
}

void Extent::ResetPtr() {
    _bitIdx = 0;
    _blockIdx = 0;
}


bool Extent::IncPtr() {
    _bitIdx++;
    if ( _bitIdx >= blocks[_blockIdx].SizeInBits() ) {
        if (_blockIdx + 1 < blocks.size()) {
            _blockIdx++;
            _bitIdx = 0;
        }else {
            return false;
        }
    }
    return true;
}

bool Extent::SetNextBit(ExtentBit bit) {
    if( _blockIdx >= blocks.size() ){
        throw out_of_bound();
    }
    if( _bitIdx >= blocks[_blockIdx].SizeInBits() ){
        throw out_of_bound();
    }
    SetAt(_blockIdx, _bitIdx, bit.value);
    if (!probability.empty()) {
        probability[_blockIdx][_bitIdx] = bit.prob;
    }
    return IncPtr();
}

bool Extent::GetNextBit(ExtentBit& nextBit) {
    if( _blockIdx >= blocks.size() ){
        throw out_of_bound();
    }
    // Impossible to happen
    /* if ( _bitIdx >= blocks[_blockIdx].SizeInBits() ){
        throw out_of_bound();
    }*/
    bool bitValue = GetAt(_blockIdx, _bitIdx);
    nextBit.value = bitValue;
    if (!probability.empty()) {
        float prob = probability[_blockIdx][_bitIdx];
        nextBit.prob = prob;
    }
    return IncPtr();
}

/**
 * Write on disk the bits as character 0 and 1
 * */
void Extent::StreamOnFile(std::string_view filePath) {
    std::ofstream outFile(filePath.data());
    if(!outFile.is_open()){
        throw bad_file_path();
    }
    for(const auto& vec : blocks){
        for(std::uint64_t bIdx = 0; bIdx < vec.SizeInBits(); bIdx++){
            outFile << (vec.GetBit(bIdx) ? '1' : '0');
        }
    }
}

/**
 * Write on disk the file in binary format
 * */
void Extent::WriteOnFile(std::ofstream& fileStream, std::uint64_t szBinaryFile) {

    if(!fileStream.is_open()){
        throw bad_file_path();
    }
    if (szBinaryFile * BITS_PER_BYTE > N_BLOCKS * BLOCK_SIZE || szBinaryFile == 0){
        throw invalid_size();
    }

    /*Save index state*/
    auto oldBlockIdx = _blockIdx;
    auto oldBitIdx = _bitIdx;

    std::vector<char> buffer;
    byte_t currentByte = 0;

    _blockIdx = 0;
    _bitIdx = 0;
    ExtentBit bit{};
    for(size_t bIdx = 0; bIdx < szBinaryFile * BITS_PER_BYTE; bIdx++){
        if (bIdx > 0 && (bIdx % (BITS_PER_BYTE)) == 0) {
            buffer.emplace_back(static_cast<char>(currentByte));
            currentByte = 0;
        }
        currentByte <<= 1;
        GetNextBit(bit);
        currentByte = currentByte + bit.value;
    }
    buffer.emplace_back(static_cast<char>(currentByte));
    currentByte = 0;
    fileStream.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    /** Restore the idx state */
    _blockIdx = oldBlockIdx;
    _bitIdx =  oldBitIdx;
}

void Extent::Fill( const std::vector<byte_t>& srcData ) {
    std::uint64_t start = 0;
    for ( std::uint64_t blockIdx = 0; blockIdx < blocks.size(); blockIdx++ ) {
        std::uint32_t nBits = 0;
        if ((start + BLOCK_SIZE) < (srcData.size() * BITS_PER_BYTE)) {
            nBits = BLOCK_SIZE;
        } else if ((start + BLOCK_SIZE) >= (srcData.size() * BITS_PER_BYTE)) {
            nBits = srcData.size() * BITS_PER_BYTE - start;
        }
        if ( nBits > 0 ) {
            blocks[blockIdx].FillBuffer(srcData, start, nBits);
        }
        start += BLOCK_SIZE;
        if ( start >= (srcData.size() * BITS_PER_BYTE) ){
            break;
        }
    }
}

void Extent::RandomizeBlocks(std::uint64_t seed) {
    for (size_t bIdx = 0; bIdx < blocks.size(); bIdx++){
        blocks[bIdx].RandomizeBits(seed);
    }
}

std::vector<std::vector<int64_t>> Extent::SerializeData(Serializer& s){
    return s.Serialize(blocks);
}

void Extent::SetBlockFromString(std::uint64_t blockIdx, std::string_view block) {
    std::stringstream sstr(std::string{block});
    if (blockIdx >= N_BLOCKS) {
        throw out_of_bound();
    }
    sstr >> blocks[blockIdx];
    //TODO: Check how many bits it wrote
}

const std::vector<BitVector>& Extent::GetBlocks() {
    return blocks;
}

void Extent::ZeroingContent(){
    for (auto& block : blocks ) {
        block.ResetValues();
    }
}