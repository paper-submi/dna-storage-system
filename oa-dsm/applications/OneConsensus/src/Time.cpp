#include "Time.hpp"
using namespace std;

Time::Time(bool is_clust) : is_cluster(is_clust){}

void Time::start_time(int phase_id) { record_time(0, phase_id); }

void Time::end_time(int phase_id) {
	record_time(1, phase_id);
	if(this->history.count(phase_id)==0){
	  this->history[phase_id]=get_time_diff(phase_id);
	}
	else{
	  this->history[phase_id]+=get_time_diff(phase_id);
	}
}

double Time::get_step_time(int phase_id) {
	double t = get_time(timing[phase_id]);
	return t;
}


void Time::print_report(std::string dev, uint32_t num_candidates,
				uint32_t num_outputs, std::ostream &out_file) {

	out_file << "MainStep,Step,SubStep,Time(sec),Device" << std::endl;

	double t=0.0;

	if(this->is_cluster){
	  t = to_sec(history[cluster::total]);
	  out_file << "Total Cluster time,\t,\t," << t << ","<< std::endl;

	  t = to_sec(history[cluster::init]);
	  out_file << "\t,Initialization,\t," << t << ","<< std::endl;

	  t = to_sec(history[cluster::onejoin]);
	  out_file << "\t,OneJoin*,\t," << t << ","<< std::endl;

	  t = to_sec(history[cluster::create_indexes]);
	  out_file << "\t,Create indexes,\t," << t << ","<< std::endl;

	  t = to_sec(history[cluster::sort]);
	  out_file << "\t,\t,Sorting," << t << ","<< std::endl;

	  t = to_sec(history[cluster::dbscan]);
	  out_file << "\t,DBSCAN,\t," << t << ","<< std::endl;

	  t = to_sec(history[cluster::range_query]);
	  out_file << "\t,Range query,\t," << t << ","<< std::endl;

	  t = to_sec(history[cluster::consensus]);
	  out_file << "\t,Consensus,\t," << t << ","<< std::endl <<std::endl;

	  t = to_sec(history[cluster::save_clusters]);
	  out_file << "\t,Save clusters,\t," << t << ","<< std::endl <<std::endl;
	}

	t = to_sec(history[init::total]);
	out_file << "Initialization,\t,\t," << t << ","<< std::endl;

	t = to_sec(history[init::init_data]);
	out_file << "\t,Init Dataset,\t," << t << "," << std::endl;

	t = to_sec(history[init::init_lsh]);
	out_file << "\t,Init LSH bits,\t," << t << ","<< std::endl;

	t = to_sec(history[init::rev_lsh]);
	out_file << "\t,Init Rev LSH array,\t," << t << ","<< std::endl;

	t = to_sec(history[embed::total]);
	out_file << "Embedding,\t,\t," << t << "," << dev << std::endl;

	t = to_sec(history[embed::alloc]);
	out_file << "\t,USM allocation,\t," << t << ","<< std::endl;

	t = to_sec(history[embed::rand_str]);
	out_file << "\t,Random string generation,\t," << t << ","<< std::endl;

	t = to_sec(history[embed::measure]);
	out_file << "\t,Measurement,\t," << t << ","<< std::endl;

	t = to_sec(history[embed::compute]);
	out_file << "\t,Computing,\t," << t << ","<< std::endl;

	t = to_sec(history[lsh::total]);
	out_file << "LSH time,\t,\t," << t << ","<< std::endl;

	t = to_sec(history[buckets::total]);
	out_file << "\t,Create Buckets,\t," << t << "," << dev << std::endl;

	t = to_sec(history[buckets::allocation]);
	out_file << "\t,\t,Buckets Allocation," << t << "," << dev << std::endl;

	t = to_sec(history[buckets::measure]);
	out_file << "\t,\t,Measurement," << t << ","<< std::endl;

	t = to_sec(history[buckets::compute]);
	out_file << "\t,\t,Computing," << t << ","<< std::endl;

	t = to_sec(history[buckets::sort]);
	out_file << "\t,\t,Sort Buckets," << t << ","<< std::endl;

	t = to_sec(history[cand_init::total]);
	out_file << "\t,Candidate Initialization,\t," << t << ","<< std::endl;

	t = to_sec(history[cand_init::comp_buck_delim]);
	out_file << "\t,\t,Compute buckets delimiter," << t << ","<< std::endl;

	t = to_sec(history[cand_init::filter_buck_delim]);
	out_file << "\t,\t,Filter one element buckets," << t << ","<< std::endl;

	t = to_sec(history[cand_init::resize]);
	out_file << "\t,\t,Allocate candidate vector," << t << ","<< std::endl;

	t = to_sec(history[cand_init::scan_cand]);
	out_file << "\t,\t,Scan cand vector (write i and j)," << t << ","<< std::endl;

	t = to_sec(history[cand::total]);
	out_file << "\t,Generate Candidate,\t," << t << "," << dev << std::endl;

	t = to_sec(history[cand::measure]);
	out_file << "\t,\t,Measurement," << t << ","<< std::endl;

	t = to_sec(history[cand::compute]);
	out_file << "\t,\t,Computing," << t << ","<< std::endl;

	t = to_sec(history[cand_proc::total]);
	out_file << "\t,Candidates processing,\t," << t << ","<< std::endl;

	t = to_sec(history[cand_proc::rem_cand]);
	out_file << "\t,\t,Remove candidates," << t << ","<< std::endl;

	t = to_sec(history[cand_proc::sort_cand]);
	out_file << "\t,\t,Sort candidates," << t << ","<< std::endl;

	t = to_sec(history[cand_proc::count_freq]);
	out_file << "\t,\t,Counting frequencies," << t << ","<< std::endl;

	t = to_sec(history[cand_proc::rem_dup]);
	out_file << "\t,\t,Remove duplicates," << t << ","<< std::endl;

	t = to_sec(history[cand_proc::sort_cand_to_verify]);
	out_file << "\t,\t,Sorting candidates to verify," << t << ","<< std::endl;

	t = to_sec(history[cand_proc::filter_low_freq]);
	out_file << "\t,\t,Remove low frequencies candidates," << t << ","<< std::endl;

	t = to_sec(history[cand_proc::make_uniq]);
	out_file << "\t,\t,Removing duplicates," << t << ","<< std::endl;

	t = to_sec(history[edit_dist::total]);
	out_file << "Edit Distance,\t,\t," << t << ","<< std::endl;

	t = to_sec(history[total_alg::total]);
	out_file << "Total OneJoin time,\t,\t," << t << ","<< std::endl;

	out_file << "Number candidates,\t" << num_candidates << ",\t,\t,"<< std::endl;
	out_file << "Number output,\t" << num_outputs << ",\t,\t,"<< std::endl;
}

void Time::print_summary(uint32_t num_candidates, uint32_t num_outputs) {

	std::cout << "\n\n\nSummary:" << std::endl << std::endl;

	double t = to_sec(history[init::total]);
	std::cout << "Time init input data: " << t << std::endl;

	t = to_sec(history[embed::total]);
	std::cout << "Time PARALLEL embedding data:\t" << t << "sec" << std::endl;

	t = to_sec(history[buckets::total]);
	std::cout << "Time PARALLEL buckets generation:\t" << t << "sec"
			  << std::endl;

	t = to_sec(history[buckets::sort]);
	std::cout << "Time buckets sorting:\t" << t << "sec" << std::endl;

	t = to_sec(history[cand_init::total]);
	std::cout << "Time candidate initialization:\t" << t << "sec" << std::endl;

	t = to_sec(history[cand::total]);
	std::cout << "Time PARALLEL candidates generation:\t" << t << "sec"
			  << std::endl;

	t = to_sec(history[cand_proc::sort_cand]);
	std::cout << "Time candidates sorting (within cand-processing):\t" << t
			  << "sec" << std::endl;

	t = to_sec(history[edit_dist::total]);
	std::cout << "Time compute edit distance:\t" << t << "sec" << std::endl;

	t = to_sec(history[lsh::total]);
	std::cout << "Total time parallel join:\t" << t << "sec" << std::endl;

	t = to_sec(history[total_alg::total]);
	std::cout << "Total elapsed time :\t" << t << "sec" << std::endl;
	std::cout << "Number of candidates verified: " << num_candidates
			  << std::endl;
	std::cout << "Number of output pairs: " << num_outputs << std::endl;
}


double Time::get_time(timeinterval_t time) {
	uint64_t d = std::chrono::duration_cast<resolution_t>(time.second -
															   time.first)
			 .count();
	double t = (double)d / converter_sec;
	return t;
}

double Time::to_sec(uint64_t val){
	return (double) val/converter_sec;
}

uint64_t Time::get_time_diff(int phase_id) {
	uint64_t diff = std::chrono::duration_cast<resolution_t>(timing[phase_id].second -
															   timing[phase_id].first)
			 .count();
	return diff;
}

void Time::record_time(int i, int phase_id) {
	if (i == 0) {
	  timing[phase_id].first = std::chrono::system_clock::now();
	} else if (i == 1) {
	  timing[phase_id].second = std::chrono::system_clock::now();
	}
}

