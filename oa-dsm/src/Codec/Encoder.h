#ifndef CONSENSUS_ENCODER_H
#define CONSENSUS_ENCODER_H

#include "Codec.h"
#include "../DataStructure/BitVector.h"

class Encoder {
    Codec& codec;
    std::vector<Motif> motifGroup;
    std::vector<Motif> prefixGroup;
    std::vector<std::int64_t> valuesGroup;

    motif_t EncodeMotif( std::uint32_t dataValue, std::uint32_t prefixIdx );

public:
    explicit Encoder( Codec& codec );
    void Fill(std::vector<std::int64_t> &data, std::uint32_t szPrefix, std::uint32_t szMotif);
    void EncodeValuesGroup();
    void GetEncodingMotifs( std::vector<Motif>& motifs );
    void Clear();
};


#endif //CONSENSUS_ENCODER_H
