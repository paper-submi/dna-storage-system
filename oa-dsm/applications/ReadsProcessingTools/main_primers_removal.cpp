#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <string_view>
#include <limits>
#include <mutex>
#include "includes.h"
#include "Utils.h"
#include "../../src/AlignmentTools/Aligner.h"

//#define DEBUG

using namespace std;

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
    po::options_description description("PrimerRemoval");
    //init_options(description, help);
    description.add_options()("help,h", po::bool_switch(&help), "Display help")(
            "input,i", po::value<string>(), "Input filename")(
            "output,o", po::value<string>(), "Output filename")(
            "strings,s", po::bool_switch()->default_value(false), "Get primers as strings")(
            "5primer,5", po::value<string>(), "Left primer")(
            "3primer,3", po::value<string>(), "Right primer")(
            "edist,e", po::value<size_t>(), "Edit distance threshold")(
            "length,l", po::value<size_t>(), "Reference length"
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

    bool left_primer_on = false;
    bool right_primer_on = false;

    vector<string> dataset;

    vector<string> left_primer;
    vector<string> right_primer;
    size_t right_length;
    size_t max_ed=0;

    if (vm.count("input") && vm.count("output") &&
        vm.count("length")) { //&& vm.count("5primer") && vm.count("3primer") ){
        filename_reads = vm["input"].as<string>();
        output_file = vm["output"].as<string>();
        right_length = vm["length"].as<size_t>();
        max_ed = vm["edist"].as<size_t>();
    } else {
        std::cerr << description << std::endl;
        return 0;
    }

    if (!vm["strings"].as<bool>()) {
        if (vm.count("5primer")) {
            filename_primerL = vm["5primer"].as<string>();
            cout << "Reading left primer from file: " << filename_primerL << std::endl;
            import_primers(filename_primerL, left_primer);
            if (left_primer.empty()) {
                cerr << "Left primer file empty" << std::endl;
                exit(-1);
            }
            left_primer_on = true;
        }
        if (vm.count("3primer")) {
            filename_primerR = vm["3primer"].as<string>();
            cout << "Reading right primer from file: " << filename_primerR << std::endl;
            import_primers(filename_primerR, right_primer);
            if (right_primer.empty()) {
                cerr << "Right primer file empty" << std::endl;
                exit(-1);
            }
            right_primer_on = true;
        }
    } else {
        std::string leftPrimerString = vm["5primer"].as<string>();
        std::string rightPrimerString = vm["3primer"].as<string>();
        if (!leftPrimerString.empty()) {
            left_primer.emplace_back(leftPrimerString);
            left_primer_on = true;
        } else {
            std::cerr << "Left primer not specified" << std::endl;
            exit(-1);
        }
        if (!rightPrimerString.empty()) {
            right_primer.emplace_back(rightPrimerString);
            right_primer_on = true;
        } else {
            std::cerr << "Right primer not specified" << std::endl;
            exit(-1);
        }
    }

    ofstream out(output_file);

    cout << "Reading dataset" << std::endl;
    read_dataset(filename_reads, dataset);

    init_dict();

    vector<string_view> dataset_out(dataset.size());
    vector<bool> valid_reads(dataset.size(), false);

    size_t count = 0;
    cout << "Extracting references" << std::endl;
    cout << "Selected Max Edit Distance: " << max_ed << std::endl;

#ifdef DEBUG
    cout<<"Running in sequentially"<<std::endl;
    for(size_t idx=0; idx<dataset.size(); idx++){
#else
    cout << "Running in parallel" << std::endl;
    tbb::parallel_for(size_t(0), dataset.size(), [&](size_t idx) {
#endif
        string_view read_to_process = dataset[idx];
        std::int32_t clean_before_result = -1;
        std::int32_t clean_after_result = -1;
        int64_t correct_primer5;
        int64_t correct_primer3;

        if (left_primer_on) {
            clean_before_result = Aligner::KeepReadAfter(read_to_process, left_primer, correct_primer5, max_ed);
        }
        if (clean_before_result >= 0) {
            if (right_primer_on) {
                clean_after_result = Aligner::KeepReadBefore(read_to_process, right_primer, correct_primer3, max_ed);
            } else {
                read_to_process = read_to_process.substr(0, right_length);
            }
        }
        if (clean_before_result >= 0 && clean_after_result>=0) {
            dataset_out[idx] = read_to_process;
            valid_reads[idx] = true;
        }
#ifdef  DEBUG
        }
#else
    });
#endif


    cout << "End postprocessing" << std::endl;

    size_t i = 0;
    for (auto read: dataset_out) {
        if (valid_reads[i]) {
            out << read << std::endl;
        } else {
            out << "" << std::endl;
        }
        i++;
    }

    return 0;
}