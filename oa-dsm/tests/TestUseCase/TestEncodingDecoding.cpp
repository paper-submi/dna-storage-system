#include <gtest/gtest.h>
#include "../../DataStructure/Extent.h"
#include "../../Codec/Encoder.h"
#include "../../Codec/Decoder.h"

const uint32_t SZ_MOTIF = 12;
const uint32_t SZ_PREFIX = 2;
const uint32_t SZ_VALUE = 10;
const char* TABLE_PATH = ".";

static const int N_VALUES = 2000;

TEST(EncodingDecoding, TwoColumnEncodingDecoding)
{

    Codec c(SZ_MOTIF, SZ_PREFIX, TABLE_PATH)   ;
    Encoder encoder(c);
    Decoder decoder(c);

    std::vector<std::int64_t> valuesColumn1(N_VALUES);
    std::vector<std::int64_t> valuesColumn2(N_VALUES);

    std::vector<Oligo> oligos(N_VALUES);
    std::vector<Motif> motifs;

    std::vector<std::int64_t> decodedValues;

    srand(0);

    for (size_t i = 0; i < valuesColumn1.size(); i++) {
        valuesColumn1[i] = rand()%1024;
    }
    for (size_t i = 0; i < valuesColumn2.size(); i++) {
        valuesColumn2[i] = rand()%1024;
    }

    encoder.Fill(valuesColumn1, SZ_VALUE, SZ_PREFIX, SZ_MOTIF);
    encoder.EncodeValuesGroup();
    encoder.GetEncodingMotifs(motifs);

    EXPECT_EQ(motifs.size(), N_VALUES);

    for (size_t i = 0; i < motifs.size(); i++) {
        oligos[i].AppendMotif(motifs[i]);
    }

    decoder.Fill(motifs, SZ_VALUE, SZ_PREFIX, SZ_MOTIF);
    decoder.DecodeMotifsGroup();
    decoder.GetDecodedValues(decodedValues);

    for (size_t i = 0; i < valuesColumn1.size(); i++) {
        EXPECT_EQ(valuesColumn1[i], decodedValues[i]);
    }

    encoder.Fill(valuesColumn2, SZ_VALUE, SZ_PREFIX, SZ_MOTIF);
    encoder.EncodeValuesGroup();
    encoder.GetEncodingMotifs(motifs);

    decoder.Fill(motifs, SZ_VALUE, SZ_PREFIX, SZ_MOTIF);
    decoder.DecodeMotifsGroup();
    decoder.GetDecodedValues(decodedValues);

    for (size_t i = 0; i < valuesColumn1.size(); i++) {
        EXPECT_EQ(valuesColumn2[i], decodedValues[i]);
    }
}