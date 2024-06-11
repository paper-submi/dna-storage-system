#include <gtest/gtest.h>
#include <fstream>
#include "../../DataStructure/BitVector.h"


TEST(BitVector, BitVectorConstructors) {
    BitVector vec;
    EXPECT_EQ( vec.SizeInBits(), 0 );

    BitVector vec2(50);
    EXPECT_EQ( vec2.SizeInBits(), 50 );
    EXPECT_EQ( vec2.SizeInBytes(), 7 );

    vec2.Clear();
    EXPECT_EQ( vec2.SizeInBits(), 0 );
    EXPECT_EQ( vec2.SizeInBytes(), 0 );

    EXPECT_THROW( BitVector vec4(-50), std::bad_alloc );

    BitVector vec3;
    EXPECT_THROW( vec3.Resize(-50), std::bad_alloc );
}


TEST(BitVector, Fill) {
    BitVector vec;
    std::vector<byte_t> data(12, 0xFF);
    data.emplace_back(0xFE);
    vec.Resize(100);
    vec.FillBuffer(data, 0, 100);
    EXPECT_EQ( vec.SizeInBits(), 100 );
    EXPECT_EQ( vec.SizeInBytes(), 13 );
    EXPECT_EQ( vec.GetBit(99), 1 );

}
TEST(BitVector, BitVectorGetSet) {
    //std::vector<byte_t> data(100);
    std::uint64_t nBits = 100;
    BitVector data(100);

    for(size_t i = 0; i < nBits; i++){
        EXPECT_NE( data.GetBit(i), 1);
        EXPECT_EQ( data.GetBit(i), 0);
    }

    for(size_t i = 0; i < nBits; i++){
        data.SetBit(i, true);
    }

    for(size_t i = 0; i < nBits; i++){
        EXPECT_EQ( data.GetBit(i), 1);
    }

    for(size_t i = 0; i < nBits; i++){
        data.SetBit(i, false);
    }

    for(size_t i = 0; i < nBits; i++){
        EXPECT_EQ( data.GetBit(i), 0);
    }

    EXPECT_THROW(auto b = data.GetBit(101), out_of_bound);
    EXPECT_THROW(data.SetBit(102, true), out_of_bound);
}

TEST(BitVector, BitVectorGetRange) {

    BitVector vec(50);

    std::uint32_t value;
    // Set 0 on 4 bits
    vec.SetBit(0, false);
    vec.SetBit(1, false);
    vec.SetBit(2, false);
    vec.SetBit(3, false);
    value = vec.GetRangeBits(0, 4);
    EXPECT_EQ(value, 0);

    // Set 1 on 4 bits
    vec.SetBit(4, false);
    vec.SetBit(5, false);
    vec.SetBit(6, false);
    vec.SetBit(7, true);
    value = vec.GetRangeBits(4, 4);
    EXPECT_EQ(value, 1);

    // Set 2 on 4 bits
    vec.SetBit(8, false);
    vec.SetBit(9, false);
    vec.SetBit(10, true);
    vec.SetBit(11, false);
    value = vec.GetRangeBits(8, 4);
    EXPECT_EQ(value, 2);

    // Set 3 on 4 bits
    vec.SetBit(12, false);
    vec.SetBit(13, false);
    vec.SetBit(14, true);
    vec.SetBit(15, true);
    value = vec.GetRangeBits(12, 4);
    EXPECT_EQ(value, 3);

    EXPECT_THROW(auto val = vec.GetRangeBits(0, 33), out_of_bound);
    EXPECT_THROW(auto val = vec.GetRangeBits(30, 21), out_of_bound);

    vec.SetBit(46, true);
    vec.SetBit(47, false);
    vec.SetBit(48, true);
    vec.SetBit(49, true);
    value = vec.GetRangeBits(46, 4);
    EXPECT_EQ(value, 11);
    EXPECT_THROW(auto val = vec.GetRangeBits(46, 5), out_of_bound);
}

TEST(BitVector, BitVectorRandomization) {

    BitVector vec10(100);
    std::vector<byte_t> data(13, 0xFF);
    vec10.FillBuffer(data, 0, 100);
    vec10.RandomizeBits(1);
    vec10.RandomizeBits(1);

    for(size_t i=0; i<vec10.SizeInBits(); i++){
        EXPECT_EQ(vec10.GetBit(i), true);
    }
}

TEST(BitVector, InStreamOperator) {

    {
        std::ofstream ifile("file.txt");
        ifile << "1101010100101" <<std::endl;
        ifile << "1 1 0 1  0 1 0 1 0  0 1 0 1 " <<std::endl;
    }
    BitVector bits(26);
    {
        std::ifstream ifile("file.txt");
        ifile >> bits;
    }
    std::string str;
    for( std::uint64_t i = 0; i < bits.SizeInBits(); i++){
        str += bits.GetBit(i)?'1':'0';
    }
    EXPECT_STREQ(str.c_str(), "11010101001010000000000000");
}

TEST(BitVector, InStreamOperator2) {

    {
        std::ofstream ifile("file.txt");
        ifile << "1101010100101" <<std::endl;
        ifile << "1 0 0 1  0 1 1 1 1  0 0 1 1 " <<std::endl;
    }
    BitVector bits(13);
    BitVector bits2(13);
    {
        std::ifstream ifile("file.txt");
        ifile >> bits;
        ifile >> bits2;
    }

    std::string str;
    for( std::uint64_t i = 0; i < bits.SizeInBits(); i++){
        str += bits.GetBit(i)?'1':'0';
    }
    EXPECT_STREQ(str.c_str(), "1101010100101");

    str="";
    for( std::uint64_t i = 0; i < bits2.SizeInBits(); i++){
        str += bits2.GetBit(i)?'1':'0';
    }
    EXPECT_STREQ(str.c_str(), "1001011110011");
}


TEST(BitVector, ExtractVector) {

    {
        std::ofstream ifile("file.txt");
        ifile << "1111111111111111111100000000000000000000" << std::endl;
    }

    BitVector bits(40);
    BitVector vec1(20);
    BitVector vec2(20);

    {
        std::ifstream ifile("file.txt");
        ifile >> bits;
    }

    vec1 = bits.ExtractBits(0, 20);
    vec2 = bits.ExtractBits(20, 20);

    std::string str;
    for( std::uint64_t i = 0; i < vec1.SizeInBits(); i++){
        str += bits.GetBit(i)?'1':'0';
    }
    EXPECT_STREQ(str.c_str(), "11111111111111111111");

    str="";
    for( std::uint64_t i = 0; i < vec2.SizeInBits(); i++){
        str += vec2.GetBit(i)?'1':'0';
    }
    EXPECT_STREQ(str.c_str(), "00000000000000000000");
}

TEST(BitVector, ToString) {
    BitVector data(5);
    data.SetBit(0, true);
    data.SetBit(1, false);
    data.SetBit(2, false);
    data.SetBit(3, true);
    data.SetBit(4, true);

    EXPECT_EQ( data.ToString(), "10011");
}

TEST(BitsView, GetBits) {
    std::vector<byte_t> data(5);
    data[0] = 0xFA;
    data[1] = 0xA7;
    data[2] = 0x98;
    data[3] = 0x1F;
    data[4] = 0xF9;

    BitsView v(data);

    EXPECT_EQ(v.GetBit(0), true);
    EXPECT_EQ(v.GetBit(1), true);
    EXPECT_EQ(v.GetBit(2), true);
    EXPECT_EQ(v.GetBit(3), true);
    EXPECT_EQ(v.GetBit(4), true);
    EXPECT_EQ(v.GetBit(5), false);
    EXPECT_EQ(v.GetBit(6), true);
    EXPECT_EQ(v.GetBit(7), false);

    EXPECT_THROW(v.GetBit(5*BITS_PER_BYTE+1), out_of_bound);
}