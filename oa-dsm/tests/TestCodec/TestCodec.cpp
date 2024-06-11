#include <gtest/gtest.h>
#include <fstream>
#include <experimental/filesystem>
#include "../../DataStructure/Motif.h"
#include "../../Codec/Codec.h"

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}


class Result{
public:
    std::string name;
    std::uint32_t size;
    Result(std::uint32_t size, std::string_view name) : name(name), size(size){}
};

TEST(OligoTest, CodecTableBuildingMotif12) {

    std::vector<Result> ExpectedResults = {{24900880, "table-0"},
                                           {30949848, "table-100663296"},
                                           {33418968, "table-117440512"},
                                           {33418968, "table-134217728"},
                                           {30949848, "table-150994944"},
                                           {33418968, "table-16777216"},
                                           {24900880, "table-167772160"},
                                           {33418968, "table-184549376"},
                                           {30949848, "table-201326592"},
                                           {33418968, "table-218103808"},
                                           {33418968, "table-234881024"},
                                           {24900880, "table-251658240"},
                                           {33418968, "table-33554432"},
                                           {30949848, "table-50331648"},
                                           {33418968, "table-67108864"},
                                           {24900880, "table-83886080"}};
    system("rm ./table-*");

    {
        Codec codec(12, 2, ".");
        for (auto &entry: ExpectedResults) {
            auto size = std::experimental::filesystem::file_size(entry.name);
            EXPECT_EQ(size, entry.size);
        }
    }
    system("rm ./table-*");
}

