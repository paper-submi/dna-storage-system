#ifndef PRIMERSREMOVAL_UTILS_H
#define PRIMERSREMOVAL_UTILS_H

#include <boost/program_options.hpp>
#include <string>
#include <algorithm>
#include <vector>

enum Type{
    NA, FASTQ, FASTA, READS
};
//void init_options(boost::program_options::options_description &description, bool &help);

Type get_dataset_format(std::string &input_filename);

bool get_chunk(std::ifstream& ifile, std::vector<std::string> &dataset, Type type, size_t nLines);

void read_dataset(std::string &input_filename, std::vector<std::string> &dataset);

void write_dataset(std::string &output_filename, std::vector<std::string> &result);

void import_primers(std::string filename_primer, std::vector<std::string> &primers);

void import_cluster(std::string &input_filename, std::vector<std::vector<std::string>> &clusters_set, std::int32_t MIN_PTS);

void reverse_complement(std::string& input, std::string &output);
void reverse_complement(std::string_view& input, std::string &output);

#endif //PRIMERSREMOVAL_UTILS_H
