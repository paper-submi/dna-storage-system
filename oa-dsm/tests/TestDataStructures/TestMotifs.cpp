#include <gtest/gtest.h>
#include <cmath>
#include "../../DataStructure/Motif.h"


TEST(OligoTest, MotifConstructors) {

    Motif motif("ACACCATCGAC");

    EXPECT_EQ(motif.GetLength(), 11);
    EXPECT_EQ(motif.toString(), "ACACCATCGAC");
    EXPECT_EQ(motif.GetSequence(), 283489);

    const std::string bad_oligo =  "ACTCTCATCTGATCGTACGTACGTACGATCGAC";

    EXPECT_EQ(Motif("ACGCTGCA").GetLength(), 8);
    EXPECT_EQ(Motif("ACGCTGCA", -1).GetLength(), 8);
    EXPECT_EQ(Motif("ACGCTGCA", 16).GetLength(), 16);
    EXPECT_EQ(Motif("ACGCTGCA", 16).toString(), "ACGCTGCAAAAAAAAA");

    EXPECT_THROW(auto m = Motif(bad_oligo, bad_oligo.size()), bad_length);
    EXPECT_THROW(auto m = Motif("ACGCTGCA", -7), bad_length);
    EXPECT_THROW(auto m = Motif("ACGCTGCA", 35).toString(), bad_length);
    EXPECT_THROW(auto m = Motif("ACGZCTGC"), bad_nucleotide);
}

TEST(OligoTest, MotifConstructors2) {

    Motif motif("", 16);

    EXPECT_EQ(motif.GetLength(), 16);
    EXPECT_EQ(motif.toString(), "AAAAAAAAAAAAAAAA");
    EXPECT_EQ(motif.GetSequence(), 0);
}

TEST(OligoTest, MotifSlice) {

    Motif motif("ACACCATCGAC");

    EXPECT_EQ(motif.Slice(1,3).GetLength(), 3);
    EXPECT_EQ(motif.Slice(1,3).toString(), "CAC");
    EXPECT_EQ(motif.Slice(0,4).toString(), "ACAC");

    EXPECT_EQ(motif.Slice(2, 4).toString(), "ACCA");
    EXPECT_EQ(motif.Slice(2, -1).toString(), "ACCATCGAC");

    EXPECT_THROW(auto m = motif.Slice(2, 15), bad_index);
    EXPECT_THROW(auto m = motif.Slice(12, 3), bad_index);
    EXPECT_THROW(auto m = motif.Slice(2, -2), bad_index);
    EXPECT_THROW(auto m = motif.Slice(3, 0), bad_index);
    EXPECT_THROW(auto m = motif.Slice(-1, 3), bad_index);
    EXPECT_THROW(auto m = motif.Slice(-3, 3), bad_index);
}

TEST(OligoTest, MotifAppend) {
    {
        Motif motifFirst("GT");
        Motif motifSecond("ACACCATCGAC");

        bool success = motifFirst.Append(motifSecond);

        EXPECT_EQ(success, true);
        EXPECT_EQ(motifFirst.toString(), "GTACACCATCGAC");
        EXPECT_EQ(motifFirst.GetLength(), 13);
    }

    {
        Motif motifFirst("GT");
        Motif motifSecond("ACACCATCGACGATCGTGCATGCAATCGATG");
        EXPECT_THROW( motifFirst.Append(motifSecond), bad_length );
    }
}

TEST(OligoTest, MotifAt) {

    std::string str = "ACACCATCGAC";
    Motif motif("ACACCATCGAC");

    EXPECT_EQ(motif.GetLength(), str.size() );

    for( size_t idx = 0; idx < str.size(); idx++){
        EXPECT_EQ(ValToNts(motif.At(idx)), str[idx] );
    }

    for( size_t idx = 0; idx < str.size(); idx++){
        EXPECT_EQ( motif[idx], str[idx] );
    }
}

TEST(OligoTest, MotifSetAt) {

    Motif motif("ACACCATCGAC", 11);
    motif.SetAt(10, NtsToVal('A'));

    EXPECT_EQ( ValToNts(motif.At(10)), 'A');
    EXPECT_EQ( motif.toString(), "ACACCATCGAA");

    EXPECT_THROW(motif.SetAt(11, 'A'), bad_index);
    EXPECT_THROW(motif.SetAt(-1, 'A'), bad_index);
}

TEST(OligoTest, MotifGetLastChar) {

    Motif motif("ACACCATCGAC", 11);

    EXPECT_EQ( motif.GetLastChars(6), "ATCGAC");
    EXPECT_EQ( motif.GetLastChars(1), "C");
    EXPECT_THROW( auto m = motif.GetLastChars(15), bad_length);
    EXPECT_THROW( auto m = motif.GetLastChars(0), bad_length);
    EXPECT_THROW( auto m = motif.GetLastChars(12), bad_length);
}

TEST(OligoTest, MotifCheck) {

    {
        Motif motif("ACACCCATCGAC", 12);
        EXPECT_EQ(motif.Check(), false);
    }
    {
        Motif motif("AACCATTTCGAC", 12);
        EXPECT_EQ(motif.Check(), false);
    }
    {
        Motif motif("ACAAACCACGGA", 12);
        EXPECT_EQ(motif.Check(), false);
    }
    {
        Motif motif("ACACCATCGGGA", 12);
        EXPECT_EQ(motif.Check(), false);
    }
    {
        Motif motif("AACCAACCTTCCGGAA", 16);
        EXPECT_EQ(motif.Check(), true);
    }
}

TEST(OligoTest, MotifNext) {

    Motif motif(0, 10);
    size_t i=0;

    EXPECT_EQ(i, motif.GetSequence());
    for(i=1; motif.Next(); i++){
        EXPECT_EQ(i, motif.GetSequence());
    }
    EXPECT_EQ(motif.GetSequence(), pow(2, 20)-1);
}