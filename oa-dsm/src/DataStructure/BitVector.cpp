#include <fstream>
#include <boost/random.hpp>
#include <boost/crc.hpp>
#include "BitVector.h"

/**
 * BitVector class allows to create a vector of "_nBits" bits.
 * Bits are stored as vector of bytes, and the addressable information unit is the bit.
 * */
BitVector::BitVector(size_t nBits) {
    this->_nBits = nBits;
    this->_buffer.resize((nBits + BITS_PER_BYTE - 1) / BITS_PER_BYTE );
}

bool BitVector::GetBit(std::uint64_t idx) const {
    if (idx >= this->_nBits) {
        throw out_of_bound();
    }
    size_t bytePos = idx / BITS_PER_BYTE;
    std::uint8_t bitPos = BITS_PER_BYTE - 1 - idx % BITS_PER_BYTE;
    auto b = static_cast<byte_t>(this->_buffer[bytePos]);
    return static_cast<bool>( (b >> bitPos) & static_cast<byte_t>(1) );
}

void BitVector::SetBit(std::uint64_t idx, bool value) {
    size_t bytePos = idx / BITS_PER_BYTE;
    uint8_t bitPos = BITS_PER_BYTE - 1 - idx % BITS_PER_BYTE;
    if (idx >= _nBits) {
        throw out_of_bound();
    }
    if (value) {
        _buffer[bytePos] |= (static_cast<byte_t>(1) << bitPos);
    } else {
        _buffer[bytePos] &= ~(static_cast<byte_t>(1) << bitPos);
    }
}

size_t BitVector::SizeInBits() const {
    return _nBits;
}

size_t BitVector::SizeInBytes() const {
    return this->_buffer.size();
}

void BitVector::Clear() {
    _buffer.clear();
    _nBits = 0;
}

void BitVector::ResetValues(){
    for (auto& b : _buffer){
        b=0;
    }
}

void BitVector::RandomizeBits(std::uint64_t seed){ // TODO: Remove boost randomization
    //byte_t mask = 0x0F;
    byte_t szBitGroup = 4;
    uint32_t randomized_value = 0;
    size_t current_position = 0;
    size_t moved = 0;

    auto MAX_RANDOM_VALUE = static_cast<byte_t>(pow(2, szBitGroup) - 2);
    boost::random::mt19937 eng( seed );
    boost::random::uniform_int_distribution<> distrib(1, static_cast<int>(MAX_RANDOM_VALUE));

    for (std::uint64_t j = 0; j < _nBits; j+=szBitGroup) {
        std::uint32_t data_value = this->GetRangeBits(j, szBitGroup);
        std::uint32_t rand_value = distrib(eng);
        data_value = data_value ^ rand_value;
        randomized_value <<= szBitGroup;
        randomized_value += data_value;
        moved += szBitGroup;
        if (moved == 8) {
            moved = 0;
            _buffer[current_position] = static_cast<byte_t>(0);
            _buffer[current_position] |= static_cast<byte_t>(randomized_value);
            current_position++;
        }
    }

    /* THIS IS THE BEST VERSION OF RANDOMIZATION.
     * However, for backcompatibility with old version we cannot still keep it

    for (std::uint64_t idx = 0; idx < _buffer.size(); idx++) {
        byte_t dataValue = _buffer[idx];
        byte_t lowNibble = dataValue & mask;
        byte_t highNibble = ( dataValue >> szBitGroup ) & mask;

        byte_t randomValueHigh = distrib(eng) & mask;
        byte_t randomizedDataHigh = highNibble ^ randomValueHigh;
        randomizedDataHigh <<= szBitGroup;

        byte_t randomValueLow = distrib(eng) & mask;
        byte_t randomizedDataLow = lowNibble ^ randomValueLow;

        _buffer[idx] = randomizedDataHigh + randomizedDataLow;
    }*/
}

std::uint32_t BitVector::ComputeCRC32(){
    boost::crc_32_type  crc32code;
    crc32code.process_bytes(_buffer.data(), _nBits );
    return crc32code.checksum();
}

std::uint32_t BitVector::GetRangeBits(std::uint64_t startIdx, std::uint64_t nBits) const {
    std::uint32_t rangeValue = 0;
    if( nBits >= BITS_PER_BYTE * sizeof(rangeValue) || startIdx >= _nBits ){
        throw out_of_bound();
    }

    if (startIdx + nBits > _nBits) {
        nBits = _nBits - startIdx;
    }

    size_t end = std::min(startIdx + nBits, _nBits);
    for (size_t idx = startIdx; idx < end; idx++) {
        rangeValue <<= 1;
        rangeValue += static_cast<byte_t>(this->GetBit(idx));
    }
    return rangeValue;
}

void BitVector::FillBuffer(const std::vector<byte_t>& data, std::uint64_t startIdx, std::uint64_t nBitsToCopy) {
    BitsView dataView(data);
    std::uint64_t locPtr = 0; //TODO: Optimize this bits insertion
    for (std::uint64_t bIdx = startIdx; bIdx < startIdx + nBitsToCopy; bIdx++, locPtr++) {
        SetBit(locPtr, dataView.GetBit(bIdx));
    }
}

void BitVector::Resize(size_t nBits) {
    this->_nBits = nBits;
    this->_buffer.resize((nBits + BITS_PER_BYTE - 1) / BITS_PER_BYTE );
}

std::istream &operator>>(std::istream &is, BitVector &vector) {
    std::uint64_t idx = 0;
    std::string line;
    std::getline(is, line);
    for (auto bit : line) {
        if (bit == '1' || bit == '0') {
            bool b = (bit == '1');
            vector.SetBit(idx, b);
            idx++;
            if (idx >= vector.SizeInBits()) {
                break;
            }
        }
    }
    return is;
}

BitVector BitVector::ExtractBits(std::uint64_t startIdx, std::uint64_t nBits) {
    BitVector copiedVec(nBits);
    copiedVec.FillBuffer(_buffer, startIdx, nBits);
    return copiedVec;
}

std::string BitVector::ToString() const {
    std::string out(_nBits, '0');
    for (size_t bIdx = 0; bIdx < _nBits; bIdx++) {
        out[bIdx] = GetBit(bIdx) ? '1' : '0';
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * BitsView class allows to create a "view" over the bits in a vector of bytes.
 * This means that it allowas to access the single bit rather than the byte, without allocating a BitVector
 */
BitsView::BitsView(const std::vector<byte_t>& data) : _ptr(data.data()), _len(data.size() * BITS_PER_BYTE) {}

BitsView::BitsView(const byte_t* ptr, size_t sz) : _ptr(ptr), _len(sz) {}

bool BitsView::GetBit(std::uint64_t idx){
    if (idx >= _len) {
        throw out_of_bound();
    }
    size_t bytePos = idx / BITS_PER_BYTE;
    std::uint8_t bitPos = BITS_PER_BYTE - 1 - idx % BITS_PER_BYTE;
    auto b = static_cast<byte_t>(_ptr[bytePos]);
    return static_cast<bool>( (b >> bitPos) & static_cast<byte_t>(1) );
}
