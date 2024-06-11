#include <sstream>
#include <ctime>
#include "utils.hpp"
#include "../../../src/Common/Logger.h"

namespace po = boost::program_options;

using namespace constants;
using namespace std;

bool InputData::append(string cell){
  
  size_t offset = this->oristrings.size();

  this->idx_oristrings.emplace_back(offset);

  this->oristrings.resize(this->oristrings.size()+cell.size());

  strncpy(oristrings.data() + offset, cell.c_str(),
            cell.size());

  len_oristrings.emplace_back(cell.size());
  
  this->N++;
  //offset += cell.size();
  return true;
}

const std::vector<char>::iterator InputData::operator[](std::size_t idx){
    size_t scaled_idx=this->idx_oristrings[idx];
    return this->oristrings.begin()+scaled_idx;
  }

  string InputData::operator()(std::size_t str_idx){
    return this->to_string(str_idx);
  }


char& InputData::operator()(std::size_t str_idx, std::size_t ch_idx){
  size_t scaled_idx = this->idx_oristrings[str_idx];
  if(scaled_idx+ch_idx > this->oristrings.size() || ch_idx<this->len_oristrings[str_idx]){
    cout<<"Out of bound"<<std::endl;
    exit(-1);
  }
  return this->oristrings[scaled_idx+ch_idx];
}


std::string InputData::to_string(std::size_t idx){
  size_t scaled_idx=this->idx_oristrings[idx];
  size_t len=this->len_oristrings[idx];

  return string(this->oristrings.begin()+scaled_idx, this->oristrings.begin()+scaled_idx+len);
} 


vector<string> buffers_set;

int barWidth = 70;

void init_options( po::options_description &description, bool &help ){

	description.add_options()("help,h", po::bool_switch(&help), "Display help")(
	  "read,r", po::value<string>(), "File containing input strings")(
	  "offset,o", po::value<uint32_t>(), "Starting position for embedding")(
	  "length,l", po::value<size_t>(),
              "Expected length for the reference oligos")(
      "dataset_name,n", po::value<string>(), "[optional] Name of dataset to use in the report name")(
      "verbose,v", po::bool_switch()->default_value(false), "[optional] Print all debug information"
    );

}

void print_configuration(size_t len_output,
                         size_t num_input_strings,
                         int samplingrange) {

  LOG(info) << std::left << std::setw(50)
                          << "Parameter selected:";
  LOG(info) << std::left << std::setw(50)
                          << "\tNum of strings:" << num_input_strings;
  LOG(info) << std::left << std::setw(50)
                          << "\tLen output:" << len_output;
  LOG(info) << std::left << std::setw(50)
                          << "\tSamplingrange:" << samplingrange;
  LOG(info) << std::left << std::setw(50)
                          << "\tNumber of Hash Function:" << NUM_HASH;
  LOG(info) << std::left << std::setw(50)
                          << "\tNumber of Bits per hash function:" << NUM_BITS;
  LOG(info) << std::left << std::setw(50)
                          << "\tNumber of Random Strings per input string:"
                          << NUM_STR;
  LOG(info) << std::left << std::setw(50)
                          << "\tNumber of Replication per input string:"
                          << NUM_REP;
  LOG(info) << std::left << std::setw(50)
                          << "\tEdit distance:" << K_INPUT;

};

void get_currenttime(std::string& data){
    std::time_t t = std::time(0);
    stringstream s;
    std::tm* now = std::localtime(&t);
    s << (now->tm_year + 1900) << '-'
      << (now->tm_mon + 1) << '-'
      <<  now->tm_mday << '-'
      << "h" << now->tm_hour << '-'
      << "m" << now->tm_min << '-'
      << "s" << now->tm_sec;

    data = s.str();
}

void save_parameters( std::string filename, int argc, char** argv, std::string& path, size_t ref_length, size_t offset, InputData& inputData ){ //size_t n_strings, size_t min_length, size_t max_length, size_t avg_length ){

    string data;
    get_currenttime(data);

    ofstream file_out(filename + "-" + data);

    string program_command="";

    for(int i=0; i<argc; i++){
        program_command += argv[i];
        program_command +=" ";
    }

    size_t n_strings = inputData.N;
    uint32_t max_length = *(std::max_element(inputData.len_oristrings.begin(), inputData.len_oristrings.end()) );
    uint32_t min_length = *(std::min_element(inputData.len_oristrings.begin(), inputData.len_oristrings.end()) );

    double tot=0.0;
    for(auto& l : inputData.len_oristrings){
        tot += static_cast<double>(l);
    }
    size_t avg_length = tot / static_cast<double>(n_strings);

    file_out << data << std::endl;
    file_out << "Command: " << program_command << " " << std::endl;
    file_out << "Parameters: " << std::endl;
    file_out << "Number of hash functions: " << NUM_HASH << std::endl;
    file_out << "Number of bits: " << NUM_BITS << std::endl;
    file_out << "Edit distance: " << K_INPUT << std::endl;
    file_out << "Reference length: " << ref_length << std::endl;
    file_out << "Offset: " << offset << std::endl;
    file_out << std::endl;

    file_out << "Constants file: " << std::endl;
    file_out << "#ifndef DEF_NUM_STR" << std::endl
    << "\t#define DEF_NUM_STR " << NUM_STR << std::endl
    << "#endif" << std::endl << std::endl
    << "#ifndef DEF_NUM_HASH" << std::endl
    << "\t#define DEF_NUM_HASH " << NUM_HASH << std::endl
    << "#endif" << std::endl << std::endl
    << "#ifndef DEF_NUM_BITS" << std::endl
    << "\t#define DEF_NUM_BITS " << NUM_BITS << std::endl
    << "#endif" << std::endl << std::endl
    << "#ifndef DEF_NUM_CHAR" << std::endl
    << "\t#define DEF_NUM_CHAR " << NUM_CHAR << std::endl
    << "#endif" << std::endl << std::endl
    << "#ifndef DEF_ALLOUTPUTRESULT" << std::endl
    << "\t#define DEF_ALLOUTPUTRESULT " << ALLOUTPUTRESULT << std::endl
    << "#endif" << std::endl << std::endl
    << "#ifndef DEF_SHIFT" << std::endl
    << "\t#define DEF_SHIFT " << SHIFT << std::endl
    << "#endif" << std::endl << std::endl
    << "#ifndef DEF_HASH_SZ" << std::endl
    << "\t#define DEF_HASH_SZ " << HASH_SZ << std::endl
    << "#endif" << std::endl << std::endl
    << "#ifndef DEF_K_INPUT" << std::endl
    << "\t#define DEF_K_INPUT " << K_INPUT << std::endl
    << "#endif" << std::endl << std::endl
    << "#ifndef DEF_CLU_CHUNK_SIZE" << std::endl
    << "\t#define DEF_CLU_CHUNK_SIZE " << clustering_chunk_size << std::endl
    << "#endif " << std::endl;

    file_out << std::endl;
    file_out << "Dataset: " <<std::endl;
    file_out << "Path: " << path << std::endl;
    file_out << "Number of strings: " << n_strings << std::endl;
    file_out << "Reference length: " << ref_length << std::endl;
    file_out << "Min length: " << min_length << std::endl;
    file_out << "Max length: " << max_length << std::endl;
    file_out << "Avg length: " << avg_length << std::endl;
}


void read_dataset(InputData &input_data, string filename) {

  LOG(info) << "Reading dataset..." << std::endl;
  ifstream data(filename);
  if (!data.is_open()) {
    LOG(error) << "Error opening input file" << std::endl;
    exit(-1);
  }

  string cell;
  string skip;
  
  int number_string = 0;

  bool is_fastq = false;

  while (getline(data, cell)) {

    if(cell[0]=='@' && number_string==0){
      is_fastq=true;
      LOG(info) << "FASTQ format detected" << std::endl;
    }else if(number_string==0){
      LOG(info) << "Simple string format detected" << std::endl;
    }

    if(is_fastq){
      getline(data, cell);
      getline(data, skip);
      getline(data, skip);
    }
    if (!cell.empty()) {
        input_data.append(cell);
        number_string++;
    }
  }
  assert(number_string==input_data.N);

};


void save_report(string dataset_name, OutputValues &output_val, Time &timer){
  string report_name; //= getReportFileName(device, batch_size);
  {
    ofstream out_file;
    out_file.open("time-report-" + dataset_name + ".csv",
                  ios::out | ios::trunc);

    if (out_file.is_open()) {
      timer.print_report(output_val.dev, output_val.num_candidates,
                         output_val.num_outputs, out_file);
    }
  }
};

void apply_cigar(std::string& s, AlignResult& ez, size_t ref_length){

    char op;
    uint32_t count;

    stringstream cigar_string(ez.cigar);

    int edit_mismatch = 0;
    unsigned ref_pos = 0, read_pos = 0;

    size_t tot=0;
    // AACGCGCT
    // 4M1D


    while ( cigar_string >> count >> op ) {
        // int count = ez.cigar[i] >> 4;
        // char op = "MID"[ez.cigar[i] & 0xf];
        // cigar_string << count << op;

        switch (op) {
            case 'M':
                //for (int j = 0; j < count; j++, ref_pos++, read_pos++) {
                tot+=count;
                break;
            case 'D':
                edit_mismatch += count;
                ref_pos += count;
                s.insert(tot, count, '-');
                tot += count;
                //tot -= count;
                break;
            case 'I':
                edit_mismatch += count;
                read_pos += count;
                s.erase(tot, count);

                //read.insert(tot - 1, count, '-');
                //tot += count;
                break;
            default:
                assert(0);
        }
    }

    for(size_t i=0; i<ez.pos; i++){
        s.insert(0, "-");
    }
    if(ref_length-s.size()) {
        size_t delta=ref_length-s.size();
        for (size_t i = 0; i < delta; i++) {
            s.insert(s.size(), "-");
        }
    }
}


int get_cigar(InputData& input_data, size_t ref_id, size_t read_id, AlignResult& ez, size_t right_lenght){

    const char *ptr_ref = input_data[ref_id].operator->();

    const char *ptr_read = input_data[read_id].operator->();

    size_t ref_size=input_data.len_oristrings[ref_id];
    size_t read_size=input_data.len_oristrings[read_id];


    EdlibAlignResult result = edlibAlign( ptr_read, read_size, ptr_ref, ref_size,
                                         edlibNewAlignConfig(K_INPUT, EDLIB_MODE_NW, EDLIB_TASK_PATH, NULL, 0));

    int edit_mismatch = result.editDistance;

    edlibFreeAlignResult(result);
    return edit_mismatch;
}


struct Dict{
    vector<uint8_t> d;
    Dict(){
        d.resize(255);
        d['A']=0;
        d['C']=1;
        d['G']=2;
        d['T']=3;
    }
};

struct Dict dict;

uint32_t StrToInt32(std::string_view& str){
    uint32_t res=0;

    for(auto& c:str){
        res = res<<2;
        res += dict.d[c];

    }
    return res;
}


void get_kmers( vector<kmer_t>& kmers, std::string_view& str, size_t K ){
    std::string_view str_view;
    if( str.size()<=K ) {
        return;
    }

    kmers.resize(str.size() - K + 1);

    size_t count = 0;
    for (size_t i = 0; i < str.size() - K + 1; i++) {
        str_view = str.substr(i, K);
        kmers[i] = StrToInt32(str_view);
        count++;
    }
    assert(count == (str.size() - K + 1));
}

bool kmers_simil(vector<kmer_t>& kmers1, vector<kmer_t>& kmers2, size_t thr){

    size_t count=0;

    for(auto& k1:kmers1){
        if(std::binary_search(kmers2.begin(), kmers2.end(), k1)){
            count++;
        }
    }

    if(count>thr){
        return true;
    }
    return false;
}


