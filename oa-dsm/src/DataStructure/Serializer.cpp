#include <cassert>
#include "Serializer.h"

/**
 * @param blocks: std::vector\<BitVector>
 *
 * @return std::vector\<std::vector\<std::int64_t\>>
 *
 * It is implementation-specific for the layout of the encoder.
 * It serializes all the blocks as vector of vector of values, adding indexes.
 * It is the link between the file layout and the encoder input.
 * */
std::vector<std::vector<std::int64_t>> ColumnarSerializer::Serialize(std::vector<BitVector>& dataBlocks) {
    std::vector<std::vector<std::int64_t>> serializedBlocks;
    std::uint64_t blockIdx = 0;
    std::uint64_t szDataTmp = szData - szIndex;
    std::uint64_t szIndexTmp = szIndex;
    serializedBlocks.emplace_back(SerializeVector(dataBlocks[blockIdx], szDataTmp, szIndexTmp));
    szDataTmp += szIndex;
    szIndexTmp -= szIndex;
    for (blockIdx = 1; blockIdx < dataBlocks.size(); blockIdx+=2){
        auto vec1 = SerializeVector(dataBlocks[blockIdx], szDataTmp, szIndexTmp);
        auto vec2 = SerializeVector(dataBlocks[blockIdx + 1], szDataTmp, szIndexTmp);
        serializedBlocks.emplace_back(std::vector<std::int64_t>{});
        std::move(std::begin(vec1), std::end(vec1), std::back_inserter(serializedBlocks.back()));
        std::move(std::begin(vec2), std::end(vec2), std::back_inserter(serializedBlocks.back()));
    }
    return serializedBlocks;
}

/**
 * @param BitVector& bits
 * @param uint32_t szData
 * @param uint32_t szIndex
 *
 * @return std::vector\<std::int64_t\>
 *
 * It takes in input a BitVector and serialize it as vector of int64_t values.
 * szData defines how many bits at time need to be taken from the input BitVector in order to make a value on the output
 * vector.
 * szIndex defines if the values in the output vector have to have an index of that number of bits.
 * If szIndex = 0, only the data from bits would be taken.
 * If szIndex > 0 (for example 15), 15 bits cotaining an index will be added to the data in the output values.
 *
 * Example:
 *  Adding indexes:
 *      szData = 15
 *      szIndex = 15
 *      Resulting values (15-index, 15-data)
 *  removing indexes:
 *      szData = 30
 *      szIndex = 0
 *      Resulting values (-, 30-data)
 * */
std::vector<std::int64_t> Serializer::SerializeVector(BitVector &bits, std::uint32_t szData, std::uint32_t szIndex) {
    assert(szData <= 32);
    std::vector<std::int64_t> values;

    std::uint32_t index = 0;

    for( std::uint64_t bIdx = 0; bIdx < bits.SizeInBits(); bIdx += szData ){
        std::int64_t value = (szIndex > 0 ? index : 0);
        value <<= szIndex;
        std::int64_t payload = bits.GetRangeBits(bIdx, szData);
        value += payload;
        std::int64_t reversedValue = 0;
        for (std::uint32_t i = 0; i < szData + szIndex; i++) {
            reversedValue <<= 1;
            reversedValue += (value & 0x1);
            value >>= 1;
        }
        values.emplace_back(reversedValue);
        index++;
    }
    return values;
}

/**
 * @param BitVector& bits
 * @param uint32_t szData
 * @param uint32_t szIndex
 *
 * @return std::vector\<std::int64_t\>
 *
 * It takes in input a vector of int64_t values and deserialize it as BitVector.
 * szData defines how many bits at time need to be taken from the vector.
 * szIndex defines the number of bits making an index so that these bits can be discarded.
 * If szIndex = 0 means that input values do not have any index.
 * If szIndex > 0 means that a certain number of bits need to be discarded from the input values before ending up in the
 * output BitVector.
 *
 * */
BitVector Serializer::DeserializeVector( std::vector<std::int64_t>& values, std::uint32_t szData, std::uint32_t szIndex){
    BitVector bits( values.size()*szData );
    std::uint64_t index = 0;

    std::uint64_t globalPtr = 0;

    for( std::int64_t value : values ){
        std::int64_t tmpValue = (value >> szIndex);
        for (std::uint64_t j = 0; j < szData; j++) {
            bool bit = (tmpValue >> j) & 0x1;
            bits.SetBit(globalPtr, bit);
            globalPtr++;
        }
        index++;
    }
    return bits;
}