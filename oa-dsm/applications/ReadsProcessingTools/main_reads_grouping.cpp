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

//#define ANALYSIS_ONLY 1
//#define DEBUG
#define PARALLEL
#define FORWARD 0
#define BACKWARD 1

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

std::vector<std::pair<AlignmentPosition, bool>> FindAllBestMotifsAlignment(std::string_view& read_to_process, std::vector<std::string>& motifs,/* std::vector<std::size_t>& best_motif_idxs, std::vector<std::pair<size_t,size_t>>& positions,*/ size_t MAX_ED);
std::pair<std::int64_t, std::int64_t> SelectLargestInterval(std::vector<std::pair<AlignmentPosition,bool>>& positions_left, std::vector<std::pair<AlignmentPosition,bool>>& positions_right, size_t reference_length);


//int32_t clean_reads_before_primers(std::string_view &read_to_process, std::vector<std::string> &motifs, std::size_t &best_motif_idx, size_t reference_length, size_t MAX_ED);
//int32_t clean_reads_after_primers(std::string_view &read_to_process, std::vector<std::string> &motifs, std::size_t &best_motif_idx, size_t reference_length, size_t MAX_ED);


int main(int argc, char **argv) {

    bool help{};
    namespace po = boost::program_options;
    po::options_description description("Grouping Reads");

    description.add_options()("help,h", po::bool_switch(&help), "Display help")(
            "input,i", po::value<string>(), "Input filename")(
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

    bool left_primer_on = false;
    bool right_primer_on = false;

    vector<string> dataset;
    vector<string> dataset_reversed;

    vector<string> left_primer;
    vector<string> right_primer;

    vector<string> reversed_left_primer;
    vector<string> reversed_right_primer;

    size_t max_ed = 0;
    size_t reference_length = 0;

    if (vm.count("input") && vm.count("edist") && vm.count("length")) {
        filename_reads = vm["input"].as<string>();
        max_ed = vm["edist"].as<size_t>();
        reference_length=vm["length"].as<size_t>();
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
                cerr << "Left Primer file empty" << std::endl;
                exit(-1);
            }
            left_primer_on = true;
        }
        if (vm.count("3primer")) {
            filename_primerR = vm["3primer"].as<string>();
            cout << "Reading right primer from file: " << filename_primerR << std::endl;
            import_primers(filename_primerR, right_primer);
            if (right_primer.empty()) {
                cerr << "Right Primer file empty" << std::endl;
                exit(-1);
            }
            right_primer_on = true;
        }
    }

    size_t N_LEFT_PRIMERS=left_primer.size();
    size_t N_RIGHT_PRIMERS=right_primer.size();

    size_t total_files = N_LEFT_PRIMERS * (N_RIGHT_PRIMERS==0 ? 1 : N_RIGHT_PRIMERS);
    cout << "Initializing | left primers | X | right primers | = " << total_files << " files for saving results"
         << std::endl;

    vector<ofstream> output_files(total_files);
    vector<std::string> output_files_paths(total_files);

    for (size_t i = 0; i < N_LEFT_PRIMERS; i++) {
        for (size_t j = 0; j < N_RIGHT_PRIMERS; j++) {
            output_files_paths[i * N_RIGHT_PRIMERS + j] = "L" + to_string(i) + "_R" + to_string(j);
        }
    }

    cout << "Reading dataset" << std::endl;
    auto type = get_dataset_format(filename_reads);
    ifstream ifile(filename_reads);

    size_t nLines = (size_t(2)<<20); // 1M lines
    dataset.clear();

    while (get_chunk(ifile, dataset, type, nLines)>0) {

        //read_dataset(filename_reads, dataset);
        /**
         * Reversing dataset*/
        cout << "Reversing dataset" << std::endl;
        dataset_reversed.resize(dataset.size());
        tbb::parallel_for(size_t(0), dataset.size(), [&](size_t idx) {
        //for (size_t idx = 0; idx < dataset.size(); idx++) {
            reverse_complement(dataset[idx], dataset_reversed[idx]);
        });

        init_dict();

        vector<string_view> dataset_out(dataset.size());
        vector<bool> is_reversed(dataset.size(), false);
        vector<bool> valid_reads(dataset.size(), false);
        vector<std::pair<uint32_t, uint32_t>> correct_primers_per_read(dataset.size());
        vector<std::pair<int32_t, int32_t>> ed_primers_per_read(dataset.size(), {INT_MAX, INT_MAX});

#ifdef DEBUG
        mutex mx;
        ofstream debug_file("debug_file");
#endif

        cout << "Grouping reads" << std::endl;
        cout << "Selected Max Edit Distance: " << max_ed << std::endl;

        for (size_t direction = 0; direction < 2; direction++) { // Forward and backward
            if (direction == 0) {
                cout << "Forward processing." << std::endl;
            } else {
                cout << "Backward processing." << std::endl;
            }
#ifdef PARALLEL
            cout << "Running in parallel" << std::endl;
            tbb::parallel_for(size_t(0), dataset.size(), [&](size_t idx) {
#else
                cout<<"Running sequentially"<<std::endl;
                for(size_t idx=0; idx<dataset.size(); idx++) {
#endif
                if (!dataset[idx].empty()) {
                    string_view read_to_process;

                    if (direction == FORWARD) {
                        read_to_process = dataset[idx];
                    } else if (direction == BACKWARD) {
                        read_to_process = dataset_reversed[idx];
                    }

                    int32_t clean_before_result = true;
                    int32_t clean_after_result = true;
                    vector<size_t> correct_primers5;
                    vector<size_t> correct_primers3;

                    size_t correct_pr5;
                    size_t correct_pr3;


                    vector<size_t> offsets;
                    std::vector<std::pair<size_t, size_t>> positions_left;
                    std::vector<std::pair<size_t, size_t>> positions_right;
                    std::vector<std::pair<AlignmentPosition, bool>> alnsLeft;
                    std::vector<std::pair<AlignmentPosition, bool>> alnsRight;

                    if (left_primer_on) {
                        alnsLeft = FindAllBestMotifsAlignment(read_to_process, left_primer, max_ed);
                    }
                    const int leftCount = std::count_if(alnsLeft.cbegin(), alnsLeft.cend(), [](auto &primerIdx) {
                        return primerIdx.second == true;
                    });

                    if (leftCount > 0 && right_primer_on) {
                        alnsRight = FindAllBestMotifsAlignment(read_to_process, right_primer, max_ed);
                    }

                    const int rightCount = std::count_if(alnsRight.cbegin(), alnsRight.cend(), [](auto &primerIdx) {
                        return primerIdx.second == true;
                    });
                    if (leftCount > 0 && rightCount > 0) {
                        std::pair<std::int64_t, std::int64_t> bestPrimerPair = SelectLargestInterval(alnsLeft,
                                                                                                     alnsRight,
                                                                                                     reference_length);

                        vector<string> tmp_primer5;
                        tmp_primer5.emplace_back(left_primer[bestPrimerPair.first]);

                        vector<string> tmp_primer3;
                        tmp_primer3.emplace_back(right_primer[bestPrimerPair.second]);

                        std::string readToShrink(read_to_process);
                        int64_t tmpIdx = 0;
                        if (left_primer_on) {
                            clean_before_result = Aligner::CleanReadBefore(readToShrink, tmp_primer5, tmpIdx, max_ed);
                        }
                        if (clean_before_result >= 0 && right_primer_on) {
                            clean_after_result = Aligner::CleanReadAfter(readToShrink, tmp_primer3, tmpIdx, max_ed);
                        }

                        valid_reads[idx] = true;

                        if (clean_before_result < ed_primers_per_read[idx].first &&
                            clean_after_result < ed_primers_per_read[idx].second) {
#ifdef ANALYSIS_ONLY
                            dataset_out[idx] = dataset[idx];
#else
                            dataset_out[idx] = read_to_process;
#endif
                            ed_primers_per_read[idx].first = clean_before_result;
                            ed_primers_per_read[idx].second = clean_after_result;
                            correct_primers_per_read[idx].first = bestPrimerPair.first;//correct_primers5[bestPrimerPair.first];//correct_pr5;
                            correct_primers_per_read[idx].second = bestPrimerPair.second;//correct_primers3[bestPrimerPair.second];//correct_pr3;
                        }
                    }
#ifdef DEBUG
                    {
#ifdef PARALLEL
                        unique_lock<mutex> lk(mx);
#endif
                        debug_file << "B: " << dataset[idx] << std::endl;
                        debug_file << "A: " << dataset_out[idx] << std::endl;
                    }
#endif
                }
#ifdef PARALLEL
            });
#else
            }
#endif
            /**
             * Don't change the primers,
             * reverse the reads only,
             * take the min edit distance between the two passes.
             *
             * Increase the edit distance threshold,
             * save anyway the edit distance for the primers
             *
             **/
        }
        cout << "End preprocessing" << std::endl;

        size_t MAX_FILE_OPEN_CONCURRENTLY = 512;
        size_t file_open_counter = 0;
        size_t i = 0;
        for (auto read: dataset_out) {
            if (valid_reads[i]) {
                auto primers_idx = correct_primers_per_read[i];
                auto fileIdx = primers_idx.first * right_primer.size() + primers_idx.second;

                while (!output_files[fileIdx].is_open()) {
                    output_files[fileIdx].open(output_files_paths[fileIdx], std::ios_base::app);
                    file_open_counter++;
                    if (!output_files[fileIdx].is_open()) {
                        cerr << "Cannot open the file" << std::endl;
                        exit(-1);
                    }
                    if (file_open_counter >= MAX_FILE_OPEN_CONCURRENTLY) {
                        for (auto &f: output_files) {
                            f.close();
                        }
                        file_open_counter = 0;
                    }
                }
                output_files[fileIdx] << read << std::endl;
            }
            i++;
        }
    }
    return 0;
}

std::vector<std::pair<AlignmentPosition, bool>> FindAllBestMotifsAlignment(std::string_view& read_to_process, std::vector<std::string>& motifs,/* std::vector<std::size_t>& best_motif_idxs, std::vector<std::pair<size_t,size_t>>& positions,*/ size_t MAX_ED) {
    std::vector<std::pair<AlignmentPosition, bool>> alnSet;
    std::int32_t MIN_ED = INT32_MAX;
    for (auto& mtf : motifs) {
        auto aln = Aligner::FindLocalAlignmentPositions(read_to_process, mtf);
        if (aln.editDistance < MIN_ED) {
            MIN_ED = aln.editDistance;
        }
        alnSet.emplace_back(aln, false);
    }
    if (MIN_ED>=0 && MIN_ED<static_cast<int32_t>(MAX_ED)) {
        std::for_each(alnSet.begin(), alnSet.end(), [MIN_ED](std::pair<AlignmentPosition, bool> &p) {
            if (p.first.editDistance == MIN_ED) {
                p.second = true;
            }
        });
    }
    return alnSet;
   /* // int min_ed = MAX_ED;
    std::pair<int32_t, int32_t> min_pos(-1, -1);
    AlignmentPosition pos;
    bool found = false;

    vector<int> edit_distances;
    vector<pair<size_t,size_t>> tmp_positions;

    size_t i = 0;
    for (auto &p: motifs) {
        int ed = 0;
        pos = Aligner::FindLocalAlignmentPositions(read_to_process, p);
        edit_distances.emplace_back(pos.editDistance);
        tmp_positions.emplace_back(pos.start, pos.end);

        /*if (ed >= 0 && ed < min_ed) {
            min_ed = ed;
            min_pos = pos;
            found = true;
        }*/
  /*      i++;
    }

    auto min_ptr = min_element(edit_distances.begin(), edit_distances.end());

    int min_ed = *min_ptr;

    if(min_ed>=0 && min_ed < MAX_ED){
        for(size_t idx=0; idx<edit_distances.size(); idx++){
            if( edit_distances[idx]==min_ed ){
                best_motif_idxs.emplace_back(idx);
                positions.emplace_back(tmp_positions[idx]);
            }
        }
        return min_ed;
    }
    return -1;*/
    /*if (found) {
        size_t start = min_pos.first < 0 ? 0 : min_pos.first;// + 1;
        //if ((read_to_process.size() - start) > 0) {
        if (read_to_process.size() > start) {
            read_to_process = read_to_process.substr(start, (read_to_process.size() - start));
            return min_ed;
        }
    }*/
    //return -1;
}

std::pair<std::int64_t, std::int64_t> SelectLargestInterval(std::vector<std::pair<AlignmentPosition,bool>>& positions_left, std::vector<std::pair<AlignmentPosition,bool>>& positions_right, size_t reference_length){
    //assert(positions_left.size()==positions_right.size());
    int64_t min_offset = INT64_MAX;
    std::int64_t min_i=-1, min_j=-1;

    for(int64_t i=0; i<positions_left.size(); i++){
        for(int64_t j=0; j<positions_right.size(); j++) {
            if (positions_left[i].second && positions_right[j].second) {
                int64_t displacement =
                        abs(static_cast<int64_t>(positions_left[i].first.end) -
                            static_cast<int64_t>(positions_right[j].first.start));
                int64_t offset = abs(static_cast<int64_t>(reference_length - displacement));

                if (offset < min_offset) {
                    //offset.emplace_back(displacement);
                    min_i = i;
                    min_j = j;
                    min_offset = offset;
                }
            }
        }
    }
    return std::pair<std::int64_t,std::int64_t>{min_i,min_j};
}