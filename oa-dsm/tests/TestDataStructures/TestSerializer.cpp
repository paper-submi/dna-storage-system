#include <gtest/gtest.h>
#include "../../DataStructure/Serializer.h"
#include "../../DataStructure/BitVector.h"
#include "../../DataStructure/Extent.h"

const std::uint32_t N_BLOCKS = 19;
const std::uint32_t BLOCK_SIZE = 332820;

TEST(ColumnarSerializer, Basics) {

    Extent extent(N_BLOCKS, BLOCK_SIZE, 0);

    /*
     * Write 0s in the first block; then write 1s in the second and the third block, and switch every values
     * (1->0 and 0->1) two blocks.
     *
     * After serialization, I expect to have a vector of BitVector having alternating 0s and 1s.
     * */
    ExtentBit nextBit{};
    nextBit.value = false;
    std::uint64_t globalPtr = 0;
    std::int64_t value = 0;
    std::uint32_t itr = 0;
    while (true) {
        nextBit.value = value;
        if( !extent.SetNextBit(nextBit) ){
            break;
        }
        globalPtr++;
        if (globalPtr % BLOCK_SIZE == 0) {
            if (itr == 0) {
                value++;
            } else if ( itr % 2 == 0) {
                value = (value == 0 ? 1 : 0);
            }
            itr++;
        }
    }
    ColumnarSerializer s(30, 15);
    std::vector<std::vector<int64_t>> serializedBlocks = extent.SerializeData(s);

    value = 0x3FFFFFFF;
    for (size_t i = 1; i < serializedBlocks.size(); i++) {
        for (std::uint64_t vIdx = 0; vIdx < serializedBlocks[i].size(); vIdx++) {
            EXPECT_EQ(serializedBlocks[i][vIdx], value);
        }
        value = (value == 0 ? 0x3FFFFFFF : 0);
    }
}

TEST(Serializer, SerializeDeserializeVector) {
    const std::uint32_t szData=15;
    const std::uint32_t szIndex=15;

    std::vector<byte_t> binary(38, 0xFF);

    for (auto& b : binary ) {
        b = rand() % 256;
    }

    BitVector data(300);

    data.FillBuffer(binary, 0, 300);

    auto values = ColumnarSerializer::SerializeVector(data, szData, szIndex);

    EXPECT_EQ(values.size(), 20);

    auto desData =  ColumnarSerializer::DeserializeVector(values, szData, szIndex);

    for (std::uint32_t idx = 0; idx < 300; idx++) {
        EXPECT_EQ(data.GetBit(idx), desData.GetBit(idx));
    }
}
