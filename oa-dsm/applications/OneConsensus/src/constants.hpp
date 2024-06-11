#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstddef>
#include <cstdint>

namespace constants {
    extern const size_t NUM_STR;
    extern const size_t NUM_HASH;
    extern const size_t NUM_BITS;
    extern const uint8_t NUM_CHAR;
    extern const bool ALLOUTPUTRESULT;
    extern const size_t SHIFT;
    extern const size_t HASH_SZ;
    extern const size_t K_INPUT;
    extern const size_t NUM_REP;

    extern const size_t clustering_chunk_size;

    extern const size_t max_buffer_size;
};

enum {cpu=0,gpu,both};
namespace alg { enum {join=1,cluster}; };

#endif //CONSTANTS_HPP