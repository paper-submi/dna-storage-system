#ifndef REFACTORINGOLIGOARCHIVE_FILE_H
#define REFACTORINGOLIGOARCHIVE_FILE_H

#include "Extent.h"
#include "BitVector.h"
#include <cstddef>
#include <string>
#include "../Common/MD5.h"

enum Format {
    Binary,
    String
};

const size_t MAX_EXTENTS_IN_MEMORY = static_cast<int>(std::thread::hardware_concurrency());

class File;

struct FileMetadata {
    friend class File;
    size_t szFile;
    //size_t szEncodedFile;
    std::int64_t randSeed=-1;
    std::string md5;
    std::string inputFilename;
    std::string ext;
    std::uint64_t nExtents;
};

struct SeekPosition {
public:
    std::int64_t _extentIdx = -1;
    std::int64_t _blockIdx = -1;
    std::int64_t _bitIdx = -1;
};


class File {
    const std::uint32_t N_BLOCKS_PER_EXTENT = 0;
    const std::uint32_t BLOCK_SIZE = 0;

    std::ifstream fileSink;
    FileMetadata metadata;
    std::vector<Extent> data;
    std::int64_t lastWrittenBlock = -1;

    //size_t MAX_MEMORY_ALLOCATED = (static_cast<size_t>(1) << 30); // 8GB
    size_t nProcessedBytes = 0;
    int64_t nRemainingBytes = 0;

public:

    explicit File(std::uint32_t N_BLOCKS_PER_EXTENT, std::uint32_t BLOCK_SIZE);

    bool Open(std::string_view filePath);

    void Close();

    bool IsEOF();

    /**
     * It checks if the file stream is open.
     * If open, it:
     *  1. reads the binary;
     *  2. allocates the extents;
     *  3. set metadata;
     *  4. fills contiguously the extents/blocks with the bits coming from the binary:
     *      i.e. it fills the whole block until the latter is full; then the whole extent.
     */
    //template<typename T>
    bool Read();

    std::vector<std::string> BlocksToStrings();

    void AppendBlock(std::string_view binaryStringBlock);;

    void Write(std::string_view filePath);

    size_t GetSzFile() const;
    int64_t GetRandSeed() const;
    const std::string &GetMd5() const;
    const std::string &GetInputFilename() const;
    const std::string &GetExt() const;
    uint64_t GetNExtents() const;
    void SetSzFile(size_t size);
    void SetRandSeed(const std::string& randSeed);
    void SetMd5(const std::string &md5);
    void SetInputFilename(const std::string &inputFilename);
    void SetExt(const std::string &ext);
    void SetNExtents(std::uint64_t nExtents);

    /**
     * Randomize the blocks in the extents, to avoid blocks with long series of zeros
     */
    void Randomize();

    /**
     * Import metadata from another file
     * */
    void ImportMetadata(File &file);

    std::vector<Extent> &GetExtentsList();
};


#endif //REFACTORINGOLIGOARCHIVE_FILE_H
