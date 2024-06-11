#ifndef REFACTORINGCODEC_SERIALIZER_H
#define REFACTORINGCODEC_SERIALIZER_H

#include <vector>
#include "BitVector.h"

class bad_size : public std::exception{
public:
    [[nodiscard]] virtual const char *what() const throw() {
        return "Index size and data size are not multiple of block size.";
    }
};

class Serializer {
protected:
    std::uint64_t szData;
    std::uint64_t szIndex;
public:
    Serializer(std::uint64_t szData, std::uint64_t szIndex) : szData(szData), szIndex(szIndex) {}
    virtual std::vector<std::vector<std::int64_t>> Serialize(std::vector<BitVector> &dataBlocks) = 0;
    static std::vector<std::int64_t> SerializeVector( BitVector& bits, std::uint32_t szData = 30, std::uint32_t szIndex = 0 );
    static BitVector DeserializeVector( std::vector<std::int64_t>& values, std::uint32_t szData = 30, std::uint32_t szIndex = 0);
};

class ColumnarSerializer : public Serializer {
public:
    ColumnarSerializer(std::uint64_t szData, std::uint64_t szIndex) : Serializer(szData, szIndex) {}
    std::vector<std::vector<std::int64_t>> Serialize(std::vector<BitVector>& dataBlocks) override;
};

#endif //REFACTORINGCODEC_SERIALIZER_H
