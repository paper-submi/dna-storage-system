#include <fstream>
#include <iostream>
#include "Utils.h"

using namespace std;
namespace po = boost::program_options;


/*void init_options(po::options_description &description, bool &help) {

    description.add_options()("help,h", po::bool_switch(&help), "Display help")(
            "input,i", po::value<string>(), "Input filename")(
            "output,o", po::value<string>(), "Output filename")(
            "strings,s", po::bool_switch()->default_value(false), "Get primers as strings")(
            "5primer,5", po::value<string>(), "Left primer")(
            "3primer,3", po::value<string>(), "Right primer")(
            "length,l", po::value<size_t>(), "Reference length"
    );

}*/

Type get_dataset_format(std::string &input_filename) {
    auto type = NA;
    {
        ifstream input(input_filename);
        std::string read;
        getline(input, read);
        if (read[0] == '@') {
            type = FASTQ;
            cout << "FASTQ format detected" << std::endl;
        } else if (read[0] == '>') {
            type = FASTA;
            cout << "FASTA format detected" << std::endl;
        } else {
            if (read[0] == 'A' || read[0] == 'C' || read[0] == 'G' || read[0] == 'T') {
                type = READS;
                cout << "Simple reads format detected" << std::endl;
            }
        }
    }
    return type;
}

bool get_chunk(std::ifstream& ifile, std::vector<std::string> &dataset, Type type, size_t nLines) {
    dataset.clear();
    if (type == READS) {
        dataset.resize(nLines);
        {
            std::string read;
            size_t read_ptr = 0;
            while (getline(ifile, read)) {
                dataset[read_ptr] = read;
                read_ptr++;
                if (read_ptr == dataset.size()) {
                    break;
                }
            }
            if (read_ptr>0) {
                dataset.resize(read_ptr);
            }
            else {
                dataset.clear();
            }
        }
    } else if (type == FASTQ) {
        dataset.resize(nLines/4);
        {
            std::string read;
            size_t read_counter = 0;
            size_t read_ptr = 0;

            while (getline(ifile, read)) {
                if (read_counter % 4 == 1) {
                    dataset[read_ptr] = read;
                    read_ptr++;
                    if (read_ptr == dataset.size()) {
                        break;
                    }
                }
                read_counter++;
            }
            if (read_ptr>0) {
                dataset.resize(read_ptr);
            }
            else {
                dataset.clear();
            }
        }
    } else if (type == FASTA) {
        dataset.resize(nLines/2);
        {
            std::string read;
            size_t i = 0;
            size_t read_ptr = 0;
            while (getline(ifile, read)) {
                if (i % 2 == 1) {
                    dataset[read_ptr] = read;
                    read_ptr++;
                    if (read_ptr == dataset.size()) {
                        break;
                    }
                }
                i++;
            }
            if (read_ptr>0) {
                dataset.resize(read_ptr);
            }
            else {
                dataset.clear();
            }
        }
    }
    return dataset.size();
}

void read_dataset(std::string &input_filename, std::vector<std::string> &dataset) {

    auto type = NA;
    {
        ifstream input(input_filename);
        std::string read;
        getline(input, read);
        if (read[0] == '@') {
            type = FASTQ;
            cout << "FASTQ format detected" << std::endl;
        } else if (read[0] == '>') {
            type = FASTA;
            cout << "FASTA format detected" << std::endl;
        } else {
            if (read[0] == 'A' || read[0] == 'C' || read[0] == 'G' || read[0] == 'T') {
                type = READS;
                cout << "Simple reads format detected" << std::endl;
            }
        }
    }
    size_t count = 0;
    {  // Count the line in a file
        ifstream input(input_filename);
        std::string read;
        while (getline(input, read)) {
            count++;
        }
    }
    if (type == READS) {
        dataset.resize(count);
        {
            ifstream input(input_filename);
            std::string read;
            size_t i = 0;
            while (getline(input, read)) {
                dataset[i] = read;
                i++;
            }
        }
    } else if (type == FASTQ) {
        dataset.resize(count/4);
        {
            ifstream input(input_filename);
            std::string read;
            size_t read_counter = 0;
            size_t read_ptr = 0;

            while (getline(input, read)) {
                if (read_counter % 4 == 1) {
                    dataset[read_ptr] = read;
                    read_ptr++;
                }
                read_counter++;
            }
        }
    } else if (type == FASTA) {
        dataset.resize(count/2);
        {
            ifstream input(input_filename);
            std::string read;
            size_t i = 0;
            while (getline(input, read)) {
                if (i % 2 == 1) {
                    dataset[i] = read;
                }
                i++;
            }
        }
    }
}

void write_dataset(std::string &output_filename, std::vector<std::string> &result) {
    size_t count = 0;

    ofstream output(output_filename);

    for (auto &s: result) {
        output << s << std::endl;
    }
}


void import_primers(string filename_primer, vector<string> &primers) {
    ifstream input(filename_primer);
    if (!input.is_open()) {
        std::cerr << "Error in opening file " << filename_primer << std::endl;
        exit(-1);
    }
    string s;
    while (getline(input, s)) {
        primers.emplace_back(s);
    }
}

void import_cluster(std::string &input_filename, std::vector<std::vector<std::string>> &clusters_set, std::int32_t MIN_PTS) {

    ifstream input(input_filename);
    std::string read;

    string n;
    size_t N;

    while (input >> N) {
        //N=stoul(n);
        if (N > MIN_PTS) {
            clusters_set.emplace_back(vector<string>(N, ""));
            for (size_t rIdx = 0; rIdx < N; rIdx++ ) { //clusters_set.back()) {
                input>>clusters_set.back()[rIdx];
            }
        } else {
            std::string read;
            for (size_t i = 0; i < N; i++) {
                input >> read;
            }
        }
    }
}

void reverse_complement(std::string& input, std::string &output){
    std::vector<char> map(256);
    map['A']='T';
    map['C']='G';
    map['G']='C';
    map['T']='A';

    output="";
    for(int64_t i=input.size()-1; i>=0; i--){
        output+=map[input[i]];
    }
}

void reverse_complement(std::string_view& input, std::string &output){
    std::vector<char> map(256);
    map['A']='T';
    map['C']='G';
    map['G']='C';
    map['T']='A';

    output="";
    for(int64_t i=input.size()-1; i>=0; i--){
        output+=map[input[i]];
    }
}