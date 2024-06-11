#ifndef CONSENSUS_DECODER_H
#define CONSENSUS_DECODER_H

#include "Codec.h"
#include "../DataStructure/BitVector.h"

class Decoder {
    Codec& codec;
    std::vector<Motif> motifGroup;
    std::vector<Motif> prefixGroup;
    std::vector<std::int64_t> decodedGroup;

    std::int64_t DecodeMotif(Motif& motif, Motif& prefix);

public:
    explicit Decoder( Codec& codec );

    void Fill(std::vector<Motif> &motifs, std::uint32_t szPrefix);
    void ReplaceLastDecodedMotifs(std::vector<Motif>& motifs);
    void DecodeMotifsGroup();
    void GetDecodedValues( std::vector<std::int64_t>& decodedValues );
};


#endif //CONSENSUS_DECODER_H
