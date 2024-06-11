#ifndef CONSENSUS_CONSTANTS_H
#define CONSENSUS_CONSTANTS_H

#include <string>
#include "CodecConfiguration.h"
#include "FileConfiguration.h"

extern CodecConfiguration CodecConfig;
extern FileConfiguration FileConfig;


namespace Settings {
    const uint32_t N_PREFIXES = 16;
    const uint32_t N_THREADS = N_PREFIXES;
    const uint32_t PREFIX_SIZE = 2;
    const uint32_t BITVALUE_SIZE = 30; // Number of bits making a value to encode
    const uint32_t IDX_OFFSET = 0; // Means, we found index in the first sub-oligo
    const uint32_t MAX_IDX_SIZE = 64; // Max number of bits an index can have
    const size_t NTS = 16; // Number of nucleotides making a motif

    const double LDPC_ALPHA = 0.3; // Redundancy factor for LDPC
    const size_t INDEX_LEN = 15; // Number of bits reserved to the index, i.e. half motif

    const size_t LDPC_DIM = 256000; // Initial size for a LDPC block
    const size_t EXTENDED_LDPC_DIM = static_cast<size_t>((1.0 + LDPC_ALPHA) *
                                                         256000 ); // Size of LDPC block after each block has been encoded
    const size_t FINAL_SIZE_LDPC_BLOCK =
            ((Settings::EXTENDED_LDPC_DIM + Settings::BITVALUE_SIZE - 1) / Settings::BITVALUE_SIZE) *
            Settings::BITVALUE_SIZE; // Size of LDPC block that is multiple of INDEX_LEN

    const size_t RAND_LEN = 4;
    const size_t CHUNK_LEN = 1920;
    const double EPS = 0.04;
    const size_t NUMBER_OF_MOTIFS = (CHUNK_LEN / BITVALUE_SIZE);
    const size_t NUMBER_BLOCKS_PER_EXTENT = (NUMBER_OF_MOTIFS*2) - 1; // Number of blocks contained in one extent (number of rows in one extent)

}

#endif //CONSENSUS_CONSTANTS_H
