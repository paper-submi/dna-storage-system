#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <fstream>
#include <iomanip>

#define BOOST_LOG_DYN_LINK 1
#define BOOST_ALL_DYN_LINK 1
#include <boost/program_options.hpp>

#include "constants.hpp"
#include "Time.hpp"
#include "../../../src/AlignmentTools/edlib/include/edlib.h"


#define NUMREPCHARS(len_output) (len_output * NUM_REP)
#define NUMSTRCHARS(len_output) (NUMREPCHARS(len_output) * NUM_STR)
#define ABSPOS(i,j,k,m,len_output) static_cast<unsigned int>(i * NUMSTRCHARS(len_output) + j * NUMREPCHARS(len_output) + k * len_output + m)
#define ABSPOS_P(j,t,d,len) static_cast<unsigned int>(j*NUM_CHAR*len +t*len+d)

using kmer_t = uint16_t;

class InputData{
	public:
		std::vector<char> oristrings;
		std::vector<size_t> idx_oristrings;
		std::vector<uint32_t> len_oristrings;
		size_t N=0;

	
	bool append(std::string cell);
	const std::vector<char>::iterator operator[](std::size_t idx);
	char& operator()(std::size_t str_idx, std::size_t ch_idx);

	std::string operator()(std::size_t str_idx);
	
	std::string to_string(std::size_t idx);
	
	//friend ostream& operator<<(ostream& os, const char& dt);

};


struct OutputValues{
	std::string dev;
	size_t num_candidates;
	size_t num_outputs;
	OutputValues():dev(""), num_candidates(0),num_outputs(0){}
};


struct candidate_t {
	uint32_t idx_str1;
	uint32_t len_diff;
	uint32_t idx_str2;
	uint16_t rep12_eq_bit;
	candidate_t(): idx_str1(0), len_diff(0), idx_str2(0), rep12_eq_bit(0) {}
	candidate_t(uint32_t idx_str1, uint32_t len_diff, uint32_t idx_str2, uint8_t rep12_eq_bit):
		idx_str1(idx_str1), len_diff(len_diff), idx_str2(idx_str2), rep12_eq_bit(rep12_eq_bit) {}
	bool operator<(const candidate_t& rhs) const {return ( (idx_str1 < rhs.idx_str1)
			|| (idx_str1 == rhs.idx_str1 && idx_str2 < rhs.idx_str2)
			|| (idx_str1 == rhs.idx_str1 && idx_str2 == rhs.idx_str2 && rep12_eq_bit < rhs.rep12_eq_bit)); }

	bool operator!=(const candidate_t& rhs) const {
		return !(idx_str1 == rhs.idx_str1 && idx_str2 == rhs.idx_str2 && len_diff == rhs.len_diff && rep12_eq_bit == rhs.rep12_eq_bit);
	};
	bool operator==(const candidate_t& rhs) const {
			return (idx_str1 == rhs.idx_str1 && idx_str2 == rhs.idx_str2 && len_diff == rhs.len_diff && rep12_eq_bit == rhs.rep12_eq_bit);
	};
};

struct buckets_t {
	uint32_t idx_rand_str;
	uint32_t idx_hash_func;
	uint32_t hash_id;
	uint32_t idx_str;
	uint32_t idx_rep;
	buckets_t():idx_rand_str(0), idx_hash_func(0), hash_id(0), idx_str(0), idx_rep(0){}
	buckets_t(uint32_t id_rand_string, uint32_t id_hash_func, uint32_t hash_id, uint32_t id_string, uint32_t id_rep ):
		idx_rand_str(id_rand_string), idx_hash_func(id_hash_func),
		hash_id(hash_id), idx_str(id_string), idx_rep(id_string){}
	bool operator<(const buckets_t& rhs) const {return ( (idx_rand_str < rhs.idx_rand_str)
		|| (idx_rand_str == rhs.idx_rand_str && idx_hash_func < rhs.idx_hash_func)
		|| (idx_rand_str == rhs.idx_rand_str && idx_hash_func == rhs.idx_hash_func && hash_id < rhs.hash_id)
		|| (idx_rand_str == rhs.idx_rand_str && idx_hash_func == rhs.idx_hash_func && hash_id == rhs.hash_id && idx_str < rhs.idx_str)
		|| (idx_rand_str == rhs.idx_rand_str && idx_hash_func == rhs.idx_hash_func && hash_id == rhs.hash_id && idx_str == rhs.idx_str && idx_rep < rhs.idx_rep) ); }
};

struct AlignResult{
    std::string cigar="";
    size_t pos=0;
    bool valid = false;
    AlignResult(std::string cigar="", size_t pos=0 ) : cigar(cigar), pos(pos) {};
};


void init_options( boost::program_options::options_description &description, bool &help);
void read_dataset(InputData &input_data, std::string filename);
void print_configuration(size_t len_output, size_t num_input_strings, int samplingrange);
void save_report(std::string dataset_name, OutputValues &output_val, Time &timer);
void save_parameters( std::string filename, int argc, char** argv, std::string& path, size_t ref_length, size_t offset, InputData& inputData );

int get_cigar(InputData& input_data, size_t ref_id, size_t read_id, AlignResult& ez, size_t right_length);
void apply_cigar(std::string& s, AlignResult& ez, size_t ref_length);

void get_kmers(std::vector<kmer_t>& kmers, std::string_view& str, size_t K=10);
bool kmers_simil(std::vector<kmer_t>& kmers1, std::vector<kmer_t>& kmers2, size_t thr);


#endif /* SRC_UTILS_HPP_ */

