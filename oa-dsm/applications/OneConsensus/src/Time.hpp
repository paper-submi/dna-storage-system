#ifndef TIME_HPP_
#define TIME_HPP_

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <tuple>


using timepoint_t = std::chrono::system_clock::time_point;
using timeinterval_t = std::pair<timepoint_t, timepoint_t>;

namespace init {
enum { total, init_data, init_lsh, rev_lsh, end };
}
namespace embed {
enum { total = init::end + 1, alloc, rand_str, measure, compute, end };
}
namespace buckets {
enum { total = embed::end + 1, allocation, measure, compute, sort, end };
}
namespace cand_init {
enum {
  total = buckets::end + 1,
  comp_buck_delim,
  filter_buck_delim,
  resize,
  scan_cand,
  end
};
}
namespace cand {
enum { total = cand_init::end + 1, measure, compute, end };
}
namespace cand_proc {
enum {
  total = cand::end + 1,
  rem_cand,
  sort_cand,
  count_freq,
  rem_dup,
  sort_cand_to_verify,
  filter_low_freq,
  make_uniq,
  end
};
}
namespace edit_dist {
enum { total = cand_proc::end + 1, end };
}
namespace lsh {
enum { total = edit_dist::end + 1, end };
}
namespace total_alg {
enum { total = lsh::end + 1, end };
}
namespace cluster {
enum {
  total = total_alg::end + 1,
  init,
  onejoin,
  create_indexes,
  sort,
  dbscan,
  range_query,
  consensus,
  save_clusters,
  end
};
}

using resolution_t = std::chrono::nanoseconds;

const double converter_sec = 1000000000.0;

class Time {

	  std::map<int, timeinterval_t> timing;
	  std::map<int,uint64_t> history;

	  timepoint_t t = std::chrono::system_clock::now();
	  bool is_cluster;

	  double get_time(timeinterval_t time);
	  double to_sec(uint64_t val);
	  uint64_t get_time_diff(int phase_id);
	  void record_time(int i, int phase_id);

public:

	explicit Time(bool is_clust);
	void start_time(int phase_id);
	void end_time(int phase_id);
	double get_step_time(int phase_id);
	void print_report(std::string dev, uint32_t num_candidates,
					uint32_t num_outputs, std::ostream &out_file = std::cout);
	void print_summary(uint32_t num_candidates, uint32_t num_outputs);
};

#endif /* SRC_TIME_HPP_ */
