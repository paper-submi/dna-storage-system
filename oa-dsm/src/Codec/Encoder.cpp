#include <cassert>
#include "Encoder.h"
#include "../Common/Logger.h"

Encoder::Encoder( Codec& codec ) : codec(codec){}

motif_t Encoder::EncodeMotif(std::uint32_t dataValue, std::uint32_t prefixIdx){
    if (prefixIdx >= codec.GetEncodingTables().size() ) {
        throw bad_prefix();
    }
    if( dataValue >= codec.GetEncodingTables()[prefixIdx].size() ) {
        throw bad_value();
    }
    return codec.GetEncodingTables()[prefixIdx][dataValue];
}

void Encoder::Fill(std::vector<std::int64_t> &data, std::uint32_t szPrefix, std::uint32_t szMotif) {
    /*if ((data.SizeInBits() % szValue) != 0) {
        throw invalid_data_size();
    }*/

    std::uint32_t nValues = data.size();//.SizeInBits() / szValue;

    if ( motifGroup.empty() ) {
        prefixGroup.resize(nValues, Motif(0, szPrefix));
    }else{
        assert(nValues == motifGroup.size());
        assert(nValues == prefixGroup.size());
        for (std::uint64_t idx = 0; idx < nValues; idx++) {
            std::uint32_t currentLength = motifGroup[idx].GetLength();
            prefixGroup[idx] = motifGroup[idx].Slice(currentLength - szPrefix, szPrefix);
        }
    }

    motifGroup.resize(nValues, Motif(0, szMotif));
    valuesGroup = data;
    assert(nValues == motifGroup.size());
}

void Encoder::EncodeValuesGroup(){
    tbb::parallel_for(size_t(0), motifGroup.size(), [&](size_t idx) {
    //for (size_t idx = 0; idx < motifGroup.size(); idx++){
        motif_t motifValue = EncodeMotif(valuesGroup[idx], prefixGroup[idx].GetSequence());
        std::uint32_t szMotif = motifGroup[idx].GetLength();
        motifGroup[idx] = Motif(motifValue, szMotif);
    });
}

void Encoder::GetEncodingMotifs(std::vector<Motif> &motifs) {
    motifs=motifGroup;
}

void Encoder::Clear() {
    motifGroup.clear();
    valuesGroup.clear();
    prefixGroup.clear();
}