#include <gtest/gtest.h>
#include "../../DataStructure/Motif.h"


TEST(OligoTest, OligoConstructors) {

    Oligo oligo;

    EXPECT_EQ(oligo.GetMotifLength(), 0);

    oligo.SetOligo("ACGCTGACGACTGACTACACTAGCTACGATCCAGCTGCATGCATCGAC", 16);
    EXPECT_EQ(oligo.GetMotifLength(), 16);


    EXPECT_EQ(oligo[0].toString(), "ACGCTGACGACTGACT");
    EXPECT_EQ(oligo[0].GetLength(), 16);

    EXPECT_EQ(oligo[1].toString(), "ACACTAGCTACGATCC");
    EXPECT_EQ(oligo[1].GetLength(), 16);

    EXPECT_EQ(oligo[2].toString(), "AGCTGCATGCATCGAC");
    EXPECT_EQ(oligo[2].GetLength(), 16);

    EXPECT_EQ(oligo.toString(), "ACGCTGACGACTGACTACACTAGCTACGATCCAGCTGCATGCATCGAC");
}