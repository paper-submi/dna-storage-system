#include <experimental/filesystem>
#include <boost/multiprecision/cpp_int.hpp>
#include <fstream>
#include <tbb/parallel_for.h>
#include "../Common/MD5.h"
#include "File.h"


void File::SetSzFile(size_t size) {
    metadata.szFile = size;
}

void File::SetRandSeed(const std::string& md5Str) {
    boost::multiprecision::uint256_t md5_tmp;
    std::stringstream s_hex, s_dec;

    s_hex << std::hex << md5Str;
    s_hex >> md5_tmp;
    s_dec << std::dec << md5_tmp;

    metadata.randSeed = stoull(s_dec.str().substr(0, 9));
}

void File::SetMd5(const std::string &md5Str) {
    metadata.md5 = md5Str;
}

void File::SetInputFilename(const std::string &filename) {
    metadata.inputFilename = filename;
}

void File::SetExt(const std::string &extention) {
    metadata.ext = extention;
}

void File::SetNExtents(uint64_t n) {
    metadata.nExtents = n;
}

size_t File::GetSzFile() const {
    return metadata.szFile;
}

int64_t File::GetRandSeed() const {
    return metadata.randSeed;
}

const std::string& File::GetMd5() const {
    return metadata.md5;
}

const std::string &File::GetInputFilename() const {
    return metadata.inputFilename;
}

const std::string &File::GetExt() const {
    return metadata.ext;
}

uint64_t File::GetNExtents() const {
    return metadata.nExtents;
}

void File::Write(std::string_view filePath) {
    std::ofstream file(filePath.data());

    std::uint64_t szExtentBytes = N_BLOCKS_PER_EXTENT * BLOCK_SIZE / BITS_PER_BYTE;
    std::uint64_t extIdx = 0;
    for (auto& ext : data) {
        std::uint64_t szToWrite = std::min(metadata.szFile - szExtentBytes * extIdx, szExtentBytes);
        ext.WriteOnFile(file, szToWrite);
        extIdx++;
    }

}

void File::AppendBlock(std::string_view binaryStringBlock) {
    if ( GetRandSeed() == - 1 || GetSzFile() == 0) {
        throw std::exception();
    }
    auto globalBlockIdx = lastWrittenBlock + 1;
    auto N_EXTENTS = data.size();
    if (globalBlockIdx >= 0 && static_cast<size_t>(globalBlockIdx) >= N_EXTENTS * N_BLOCKS_PER_EXTENT) {
        data.emplace_back(Extent(N_BLOCKS_PER_EXTENT, BLOCK_SIZE, GetRandSeed()));
    }
    std::uint64_t extIdx = globalBlockIdx / N_BLOCKS_PER_EXTENT;
    std::uint64_t localBlockIdx = globalBlockIdx % N_BLOCKS_PER_EXTENT;

    data[extIdx].SetBlockFromString(localBlockIdx, binaryStringBlock);
    lastWrittenBlock++;
}

std::vector<std::string> File::BlocksToStrings() {
    std::vector<std::string> stringSet;
    for (auto& ext : data) {
        for (const auto& block : ext.GetBlocks()) {
            stringSet.emplace_back(block.ToString());
        }
    }
    return stringSet;
}

bool File::Open(std::string_view filePath) {
    fileSink.open(filePath.data());
    SetInputFilename(filePath.data());
    SetSzFile(std::experimental::filesystem::file_size(std::experimental::filesystem::path{filePath}));
    auto extension = std::experimental::filesystem::path{filePath}.extension();
    std::string ext = (extension.empty() ? "None" : extension);

    /*
     * Set metadata
     * */
    SetExt(ext);
    /*
         * Read binary data
         * */
    auto szBinary = GetSzFile();
    /**
     * Allocate extents
     * */
    std::uint64_t SZ_EXTENT = (N_BLOCKS_PER_EXTENT * BLOCK_SIZE + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
    std::uint64_t N_EXTENTS = (szBinary + SZ_EXTENT - 1) / SZ_EXTENT;

    std::string StringMD5;

    MD5State MD5;
    std::vector<byte_t> binary(1<<30, 0);

    while (!fileSink.eof()) {

        fileSink.read(reinterpret_cast<char*>(binary.data()), binary.size());
        if (binary.size() != static_cast<size_t>(fileSink.gcount())) {
            binary.resize(fileSink.gcount());
        }
        UpdateMD5(binary, MD5);
    }

    StringMD5 = FinalizeMD5(MD5);
    SetMd5(StringMD5);
    SetRandSeed(GetMd5());
    SetNExtents(N_EXTENTS);
    fileSink.clear();
    fileSink.seekg(0, std::ios_base::beg);
    nRemainingBytes = GetSzFile();
    nProcessedBytes = 0;
    return fileSink.is_open();
}

void File::Close() {
    fileSink.close();
}

File::File(std::uint32_t N_BLOCKS_PER_EXTENT, std::uint32_t BLOCK_SIZE) : N_BLOCKS_PER_EXTENT(N_BLOCKS_PER_EXTENT), BLOCK_SIZE(BLOCK_SIZE) {}

void File::Randomize() {
    if ( GetRandSeed() == -1 ) {
        throw std::exception();
    }
    tbb::parallel_for(size_t(0), data.size(), [&](size_t idx){
        this->data[idx].RandomizeBlocks(GetRandSeed());
    });
    /*for (auto& ext : data) {
        ext.RandomizeBlocks(GetRandSeed());
    }*/
}

void File::ImportMetadata(File &file) {
    SetInputFilename(file.GetInputFilename());
    SetSzFile(file.GetSzFile());
    SetMd5(file.GetMd5());
    SetRandSeed(GetMd5());
    SetExt(file.GetExt());
    SetNExtents(file.GetNExtents());
}

std::vector<Extent> &File::GetExtentsList() {
    return data;
}

bool File::IsEOF() {
    return fileSink.eof();
}

bool File::Read(){
    assert((BLOCK_SIZE % BITS_PER_BYTE) == 0);
    if (!fileSink.is_open() || nRemainingBytes <= 0) {
        return false;
    }

    /**
     * Allocate extents
     * */
    auto szBinary = nRemainingBytes;
    std::uint64_t SZ_EXTENT = (N_BLOCKS_PER_EXTENT * BLOCK_SIZE + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
    std::uint64_t N_EXTENTS = (szBinary + SZ_EXTENT - 1) / SZ_EXTENT;

 //   MAX_EXTENTS_IN_MEMORY = (MAX_MEMORY_ALLOCATED + SZ_EXTENT - 1) / SZ_EXTENT; // Trunc

    Extent ext(N_BLOCKS_PER_EXTENT, BLOCK_SIZE, GetRandSeed());
    ext.ZeroingContent();

    std::uint64_t EXTENTS_TO_ALLOCATE = std::min(MAX_EXTENTS_IN_MEMORY, N_EXTENTS);

    data.resize(EXTENTS_TO_ALLOCATE, ext);

    /*
     * Set extents with binary data
     * */
    std::vector<byte_t> binary(SZ_EXTENT * EXTENTS_TO_ALLOCATE, 0);
    fileSink.read(reinterpret_cast<char*>(binary.data()), binary.size());
    for (std::uint64_t extIdx = 0; extIdx < EXTENTS_TO_ALLOCATE; extIdx++) {
        std::uint64_t szToCopy = std::min(SZ_EXTENT, binary.size() - extIdx * SZ_EXTENT);
        std::vector<byte_t> binBlock(binary.begin() + extIdx * SZ_EXTENT, binary.begin() + extIdx * SZ_EXTENT + szToCopy);
        data[extIdx].Fill(binBlock);
    }

    nProcessedBytes += binary.size();
    nRemainingBytes -= binary.size();
    return true;
};

