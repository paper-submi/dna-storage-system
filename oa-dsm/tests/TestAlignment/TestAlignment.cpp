#include <gtest/gtest.h>
#include <string_view>
#include "../../AlignmentTools/Aligner.h"

TEST(TestAligner, LocalAlignment0) {

    std::string read =  "ACGTGCATGCATGCATCGATCG";
    std::string motif = "ACGTGCATGCAT";

    AlignmentPosition alignment = Aligner::FindLocalAlignmentPositions(read, motif);
    EXPECT_EQ(alignment.start, 0);
    EXPECT_EQ(alignment.end, 12);
    EXPECT_EQ(alignment.editDistance, 0);
}

TEST(TestAligner, LocalAlignment1) {

    std::string read =  "ACGTGCATGCATGCATCGATCG";
    std::string motif =    "TGCATGCAT";

    AlignmentPosition alignment = Aligner::FindLocalAlignmentPositions(read, motif);
    EXPECT_EQ(alignment.start, 3);
    EXPECT_EQ(alignment.end, 12);
    EXPECT_EQ(alignment.editDistance, 0);
}

TEST(TestAligner, LocalAlignment2) {

    std::string read =  "ACGTGCAGATGCATCGATCG";
    std::string motif =    "TGCATGCAT";

    AlignmentPosition alignment = Aligner::FindLocalAlignmentPositions(read, motif);
    EXPECT_EQ(alignment.start, 3);
    EXPECT_EQ(alignment.end, 10);
    EXPECT_EQ(alignment.editDistance, 2);
}

TEST(TestAligner, EditDistance) {
    std::string read = "ACGTGCAGATGCATCGATCG";
    std::string motif = "TGCATGCAT";

    std::int32_t ED = Aligner::EditDistance(read, motif);
    EXPECT_EQ(ED, 11);
}

TEST(TestAligner, CleanReadsBefore) {
    std::string read = "ACGTGCAGATGCATCGATCG";
    std::vector<std::string> primers={"TGCATGCAT"};
    std::int64_t primerIdx = -1;
    auto ED = Aligner::CleanReadBefore(read, primers, primerIdx, 5);
    EXPECT_EQ(ED, 2);
    EXPECT_EQ(read, "TGCAGATGCATCGATCG");
}

TEST(TestAligner, CleanReadsAfter) {
    std::string read = "ACGTGGTGCCAGATGCATCGATCG";
    std::vector<std::string> primers={"TGCATGCAT"};
    std::int64_t primerIdx = -1;
    auto ED = Aligner::CleanReadAfter(read, primers, primerIdx, 5);
    EXPECT_EQ(ED, 2);
    EXPECT_EQ(read, "ACGTGGTGCCAGATGCAT");
}