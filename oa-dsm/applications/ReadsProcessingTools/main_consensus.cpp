#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <string_view>
#include <limits>
#include <mutex>
#include <unordered_map>
#include "includes.h"
#include "Utils.h"
#include "../../src/AlignmentTools/Aligner.h"

const size_t MAX_EDIT_DISTANCE = 5;
#define DEBUG
#define PARALLEL

void compute_consensus(std::vector<std::string> cluster, std::string &centroid, size_t MAX_ED);

using namespace std;

const int32_t NTS = 16;

std::vector<uint8_t> dict(255, 0);


void init_dict() {
    dict['A'] = 0;
    dict['C'] = 1;
    dict['G'] = 2;
    dict['T'] = 3;
}

uint32_t StrToInt32(std::string_view &str, std::vector<uint8_t> &dict) {
    uint32_t res = 0;

    for (auto &c: str) {
        res = res << 2;
        res += dict[c];

    }
    return res;
}


int main(int argc, char **argv) {

    bool help{};
    namespace po = boost::program_options;
    po::options_description description("StandaloneConsensus");

    description.add_options()("help,h", po::bool_switch(&help), "Display help")(
            "input,i", po::value<string>(), "Input filename")(
            "output,o", po::value<string>(), "Output filename")(
            "min_pts,p", po::value<std::int32_t>(), "Min number of reads in cluster"
    );

    po::command_line_parser parser{argc, argv};
    parser.options(description);
    auto parsed_result = parser.run();
    po::variables_map vm;
    po::store(parsed_result, vm);
    po::notify(vm);

    if (help) {
        std::cerr << description << std::endl;
        return 0;
    }

    std::string filename_reads;
    std::string filename_primerL;
    std::string filename_primerR;
    std::string output_file;

    std::int32_t minPts = 0;

    vector<vector<string>> clusters_set;
    vector<string> dataset_out;

    vector<string> left_primer;
    vector<string> right_primer;

    if (vm.count("input") &&
        vm.count("output")) {
        filename_reads = vm["input"].as<string>();
        output_file = vm["output"].as<string>();
    } else {
        std::cerr << description << std::endl;
        return 0;
    }
    if (vm.count("min_pts")) {
        minPts = vm["min_pts"].as<int32_t>();
    }

    ofstream out(output_file);


    cout << "Reading clusters_set" << std::endl;
    import_cluster(filename_reads, clusters_set, minPts);

    size_t size_after = clusters_set.size();

    cout << "Read " << clusters_set.size() << " clusters" << std::endl;

    dataset_out.resize(clusters_set.size());

    init_dict();

    vector<string_view> cluster_out(clusters_set.size());
    vector<bool> valid_reads(clusters_set.size(), false);

    size_t count = 0;
    cout << "Computing consensus references" << std::endl;

    size_t start = 0;
    size_t end = 16;
    size_t j = 0;
    size_t length = 256;
    size_t total_lenght = 0;

    {
        ofstream out_pts(output_file + "-pts");

        for (auto &c: clusters_set) {
            out_pts << c.size() << std::endl;
        }
    }

#ifdef PARALLEL
    cout << "Running in parallel" << std::endl;
    tbb::parallel_for(size_t(0), clusters_set.size(), [&](size_t idx) {
#else
        cout<<"Running sequentially"<<std::endl;
        for(size_t idx=0; idx<clusters_set.size(); idx++) {
#endif
        compute_consensus(clusters_set[idx], dataset_out[idx], MAX_EDIT_DISTANCE);
#ifdef PARALLEL
    });
#else
    }
#endif

    cout << std::endl;

    cout << "End postprocessing" << std::endl;

    for (auto read: dataset_out) {
        out << read << std::endl;
    }

    return 0;
}

void compute_consensus(std::vector<string> cluster, string &centroid, size_t MAX_ED) {
    size_t start = 0;
    size_t end = NTS;
    size_t total_length = 0;

    while (true) {
        unordered_map<string, int32_t> counter;

        for (auto &read: cluster) {
            if (!read.empty()) {
                end = read.size() > NTS ? NTS : std::string::npos;
                string ntsN = read.substr(start, end);
                counter[ntsN]++;
            }
        }

        auto max = counter.begin();

        if (!counter.empty()) {
            max = std::max_element(counter.begin(), counter.end(), [](const auto &p1, const auto &p2) {
                return p1.second < p2.second;
            });

            centroid += max->first;
            total_length += 16;
        }
        if (counter.empty()) {
            break;
        }

        for (size_t rIdx = 0; rIdx < cluster.size(); rIdx++) {
            std::string& read = cluster[rIdx];
            if (!read.empty()) {
                end = read.size() > NTS ? NTS : std::string::npos;
                std::string ntsN = read.substr(start, end);
                if (max->first == ntsN) {
                    if (read.size() > NTS) {
                        read = read.substr(NTS, read.size());
                    } else {
                        read = "";
                    }
                } else {
                    vector<string> primer_stub;
                    primer_stub.emplace_back(max->first);
                    std::string_view read_view = read;
                    std::int64_t p_index;
                    if (Aligner::KeepReadAfter(read_view, primer_stub, p_index, MAX_ED)) {
                        read = read_view;
                    } else {
                        read = "";
                    }
                }
            }
        }
    }

}