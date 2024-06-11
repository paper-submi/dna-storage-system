#ifndef CONSENSUS_BITVECTOR_H
#define CONSENSUS_BITVECTOR_H

#include <vector>
#include <exception>
#include <ostream>

const std::uint32_t BITS_PER_BYTE = 8;

class out_of_bound : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Index out of bound";
    }
};
class invalid_size : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Cannot allocate a bit vector of the specified size";
    }
};
/**
 * This class allows to work with bits.
 * The internal _buffer stores bits into a
 * vector<byte>.
 * A single bit at time or a range of at most
 * 32 bits can be accessed by means of the member
 * functions:
 * GetBit(), SetBit(), GetRangeBits()
 * */
using byte_t = std::uint8_t;

class BitVector {
    std::uint64_t _nBits = 0;
    std::vector<byte_t> _buffer;
public:
    explicit BitVector(size_t nBits = 0);

    void SetBit(std::uint64_t idx, bool value);
    [[nodiscard]] bool GetBit(std::uint64_t idx) const;
    [[nodiscard]] std::uint32_t GetRangeBits(std::uint64_t startIdx, std::uint64_t nBits) const;

    [[nodiscard]] size_t SizeInBits() const;
    [[nodiscard]] size_t SizeInBytes() const;
    void RandomizeBits(std::uint64_t seed);

    void Resize(size_t nBits);
    void Clear();

    void FillBuffer(const std::vector<byte_t> &data, std::uint64_t startIdx, std::uint64_t nBitsToCopy);

    void ResetValues();

    BitVector ExtractBits(std::uint64_t startIdx, std::uint64_t nBits);

    friend std::istream& operator>>(std::istream &is, BitVector &vector);

    std::string ToString() const;

    std::uint32_t ComputeCRC32();
};

class BitsView {
    const byte_t* _ptr;
    size_t _len = 0;
public:
    explicit BitsView(const std::vector<byte_t>& data);
    BitsView(const byte_t* ptr, size_t sz);

    bool GetBit(std::uint64_t idx);
};
#endif //CONSENSUS_BITVECTOR_H
