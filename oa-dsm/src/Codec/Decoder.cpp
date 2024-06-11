#include <sstream>
#include <cassert>
#include "../Common/Logger.h"
#include "Decoder.h"
#include "CodecExceptions.h"

using namespace std;

Decoder::Decoder( Codec& codec ) : codec(codec) {}


std::int64_t Decoder::DecodeMotif(Motif& motif, Motif& prefix) {

    motif_t prefixIdx = prefix.GetSequence();
    motif_t motifValue = motif.GetSequence();

    if (prefixIdx > codec.GetEncodingTables().size()) {
        throw bad_prefix();
    }

    auto first = codec.GetEncodingTables()[prefixIdx].begin();
    auto last = codec.GetEncodingTables()[prefixIdx].end();
    auto found = std::lower_bound(first, last, motifValue);

    if (found != last && motifValue >= *found) {
        return (found - first);
    }
    return static_cast<std::int64_t>(-1);
}

void Decoder::DecodeMotifsGroup() {
    tbb::parallel_for(size_t(0), motifGroup.size(), [&](size_t Idx) {
    //for(size_t Idx = 0; Idx < motifGroup.size(); Idx++){
        decodedGroup[Idx] = Decoder::DecodeMotif(motifGroup[Idx], prefixGroup[Idx]);
    });
}

void Decoder::Fill(std::vector<Motif> &motifs, std::uint32_t szPrefix) {
    std::uint32_t nMotifs = motifs.size();

    if ( motifGroup.empty() ) {
        prefixGroup.resize(nMotifs, Motif(0, szPrefix));
    }else{
        assert( nMotifs == motifGroup.size() );
        prefixGroup.clear();
        prefixGroup.resize(nMotifs, {0, szPrefix});
        for( std::uint64_t idx = 0; idx < nMotifs; idx++ ){
            std::uint32_t currentLength = motifGroup[idx].GetLength();
            prefixGroup[idx] = motifGroup[idx].Slice( currentLength - szPrefix, szPrefix);
        }
        assert( nMotifs == prefixGroup.size() );
    }

    //motifGroup.resize(nMotifs, Motif(0, szMotif));
    decodedGroup.clear();
    decodedGroup.resize(nMotifs, static_cast<std::int64_t>(-1));
    motifGroup = motifs;
}

void Decoder::ReplaceLastDecodedMotifs(std::vector<Motif>& motifs) {
    motifGroup = motifs;
}


void Decoder::GetDecodedValues( std::vector<std::int64_t>& decodedValues ) {
    decodedValues = std::move(decodedGroup);
    decodedGroup.clear();
    std::for_each(decodedValues.begin(), decodedValues.end(), [](auto& value){
       if(value==-1){
           value=0;
       }
    });
}
