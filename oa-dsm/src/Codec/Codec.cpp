#include <fstream>
#include <limits>
#include <cmath>
#include <mutex>
#include <boost/filesystem.hpp>
#include "../Common/Logger.h"
#include "../DataStructure/Motif.h"
#include "Codec.h"

namespace fs = boost::filesystem;

Codec::Codec(std::uint32_t szMotif, std::uint32_t szPrefix, std::string_view tablesPath){

    bool success = ReadTables(szMotif, szPrefix, tablesPath);
    if(!success){
        LOG(warning) << "Tables not found. Write tables with " << szMotif << std::endl;
        SaveTables(szMotif, szPrefix, tablesPath);
    }
}

size_t GetTableSize(std::string_view tablePath) {
    fs::path table(std::string{tablePath});
    return fs::file_size(table);
}

void Codec::GenerateMotifs(std::uint64_t startingMotif, std::uint64_t endingMotif, std::uint32_t szMotif,
                           std::uint32_t szPrefix,
                           std::ofstream &file) {

    Motif motif(startingMotif, szMotif + szPrefix);
    /*
     * Extract the prefix (2 most significant chars -> 4 msb)
     */
    motif_t prefix = startingMotif >> (szMotif * 2);

    /*
     * Get max values
     * Shift to left of the length of the oligo
     * Change 1s with 0s
     **/
    auto prefixMask = static_cast<motif_t>(-1);
    prefixMask <<= ( szMotif * 2 );
    prefixMask = ~prefixMask;

    for (std::uint64_t idx = startingMotif; idx < endingMotif; idx++) {

        if (motif.Check()) {
            /* Remove prefix */
            motif_t motifOnly = (motif.GetSequence() & prefixMask);
            this->encodingTables[prefix].emplace_back(static_cast<short_motif_t>(motifOnly));
        }
        motif.Next();
    }
    {
        LOG(info) << "Table " << startingMotif << " has " << this->encodingTables[prefix].size() << " entries"
                  << std::endl;
    }
    for (unsigned int &i: this->encodingTables[prefix]) {
        file.write(reinterpret_cast<char *>(&i), sizeof(i));
    }
}

/**
 * Param len must include the prefix
 * Refactoring: Len most not include anymore the prefix
 * */
bool Codec::ReadTables(std::uint32_t motifLength, std::uint32_t szPrefix, std::string_view tablesPath) {
    auto nPrefixes = static_cast<motif_t>( std::pow(4, szPrefix) );
    auto workload = static_cast<motif_t>( std::pow(4, motifLength) );

    for (uint64_t prefixIdx = 0; prefixIdx < nPrefixes; prefixIdx++) {
        std::string tableName = std::string{tablesPath} + "/table-" + std::to_string(prefixIdx * workload);
        std::ifstream tableStream(tableName, std::ios::binary);
        if (!tableStream.is_open()) {
            this->encodingTables.clear();
            return false;
        }
        this->encodingTables.emplace_back(std::vector<short_motif_t>{});
    }

    LOG(info) << "Reading tables" << std::endl;
    try{
        tbb::parallel_for(static_cast<size_t>(0), static_cast<size_t>(nPrefixes), [&](size_t prefixIdx) {

            std::string tableName = std::string{tablesPath} + "/table-" + std::to_string(prefixIdx * workload);
            std::ifstream tableStream(tableName, std::ios::binary);

            size_t szTable = GetTableSize(tableName);
            this->encodingTables[prefixIdx].resize(szTable / sizeof(short_motif_t));

            tableStream.read(reinterpret_cast<char*>(encodingTables[prefixIdx].data()), static_cast<std::streamsize>(szTable));
            if (!tableStream || tableStream.peek() != EOF) {
                throw bad_encoding_table();
            }
        });
    }catch(std::exception &e){
        LOG(error) << "Exception thrown while reading encoding tables" << std::endl;
        return false;
    }
    return true;
}

/**
 * Param len include the prefix.
 * */
void Codec::SaveTables(std::uint32_t motifLength, std::uint32_t szPrefix, std::string_view tablesPath) {

    auto nPrefixes = static_cast<std::uint64_t>( pow(4, szPrefix) );
    auto workload = static_cast<std::uint64_t>( pow(4, motifLength));

    for (uint64_t prefixIdx = 0; prefixIdx < nPrefixes; prefixIdx++) {
        this->encodingTables.emplace_back(std::vector<short_motif_t>{});
    }

    tbb::parallel_for(static_cast<size_t>(0), static_cast<size_t>(nPrefixes), [&](size_t prefixIdx) {

        std::string tableName = std::string{tablesPath} + "/table-" + std::to_string(prefixIdx * workload);
        std::ofstream tableStream(tableName, std::ios::out | std::ios::binary);

        if (!tableStream.is_open()) {
            throw tables_not_found();
        }

        this->encodingTables[prefixIdx];

        uint64_t startingMotif = prefixIdx * workload;
        uint64_t endingMotif = startingMotif + workload;

        GenerateMotifs(startingMotif, endingMotif, motifLength, szPrefix, tableStream);
    }
    );
}

void Codec::ClearTables() {
    this->encodingTables.clear();
}

const std::vector<std::vector<short_motif_t>> &Codec::GetEncodingTables() {
    return encodingTables;
}
