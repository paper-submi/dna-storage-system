#ifndef ALLSTRINGS_CODEC_H
#define ALLSTRINGS_CODEC_H

#include <string_view>
#include <exception>
#include "tbb/parallel_for.h"
#include "../DataStructure/Motif.h"
#include "CodecExceptions.h"

class Codec {
    std::vector<std::vector<short_motif_t>> encodingTables;

    void SaveTables(std::uint32_t motifLength, std::uint32_t szPrefix, std::string_view tablesPath);
    bool ReadTables(std::uint32_t motifLength, std::uint32_t szPrefix, std::string_view tablesPath);
    void GenerateMotifs(std::uint64_t startingMotif, std::uint64_t endingMotif, std::uint32_t szMotif,
                               std::uint32_t szPrefix,
                               std::ofstream &file);

public:
    explicit Codec(std::uint32_t szMotif, std::uint32_t szPrefix, std::string_view tablesPath);
    const std::vector<std::vector<short_motif_t>>& GetEncodingTables();
    void ClearTables();
};
#endif // ALLSTRINGS_CODEC_H

