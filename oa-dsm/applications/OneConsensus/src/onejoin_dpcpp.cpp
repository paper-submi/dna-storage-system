#include "onejoin.hpp"
#include <tbb/blocked_range3d.h>
#include <tbb/blocked_range2d.h>
#include "../../../src/Common/Logger.h"

using namespace std;
using namespace constants;

uint32_t samplingrange = 0; // The maximum digit to embed, the range to sample

size_t offset=0;

size_t test_batches = 2;
int num_thr = 0;

Time timer(false);

void setuplsh(vector<vector<int>> &hash_lsh, std::vector<int> &a,
              std::vector<int> &lshnumber, vector<tuple<int, int>> &rev_hash) {

  timer.start_time(init::init_lsh);

  for (int i = 0; i < NUM_HASH; i++) {
    for (int j = 0; j < NUM_BITS; j++) {
        hash_lsh[i][j] = ( rand() % ( samplingrange-offset ) ) + offset;
    }
  }

  for (int i = 0; i < NUM_BITS; i++) {
    a.push_back(rand() % (HASH_SZ - 1));
  }

  for (int i = 0; i < NUM_HASH; i++) {
    for (int j = 0; j < NUM_BITS; j++) {
      lshnumber.emplace_back(hash_lsh[i][j]);
    }
  }

  tbb::parallel_sort(lshnumber.begin(), lshnumber.end());
  lshnumber.erase(unique(lshnumber.begin(), lshnumber.end()), lshnumber.end());
  samplingrange = lshnumber[lshnumber.size() - 1];

  for (int i = 0; i < NUM_HASH; i++) {
    for (int j = 0; j < NUM_BITS; j++) {
      hash_lsh[i][j] =
          lower_bound(lshnumber.begin(), lshnumber.end(), hash_lsh[i][j]) -
          lshnumber.begin();
    }
  }
  timer.end_time(init::init_lsh);

  for(auto& e:lshnumber){
      cout<<e<<" "<<std::endl;
  }

  /**
   *  Compute the position in the embedded string for each lsh bit
   * */
  timer.start_time(init::rev_lsh);

  rev_hash.resize(lshnumber.size(), make_tuple(-1, -1));
  int k = 0;

  for (int i = 0; i < NUM_HASH; i++) {
    for (int j = 0; j < NUM_BITS; j++) {
      if (get<0>(rev_hash[hash_lsh[i][j]]) != -1) {
        // Find last pos
        int t = hash_lsh[i][j];
        while (get<1>(rev_hash[t]) != -1) {
          t = get<1>(rev_hash[t]);
        }
        rev_hash.emplace_back(make_tuple(k, -1));
        get<1>(rev_hash[t]) = rev_hash.size() - 1;
      } else {
        get<0>(rev_hash[hash_lsh[i][j]]) = k;
      }
      k++;
    }
  }
  timer.end_time(init::rev_lsh);
}



void initialize_input_data(InputData &input_data) {

  auto start = std::chrono::system_clock::now();
  /*size_t offset = 0;
  for (int i = 0; i < input_data.size(); i++) {
    idx_oristrings.emplace_back(offset);
    strncpy(oristrings.data() + offset, input_data[i].c_str(),
            input_data[i].size());
    len_oristrings.emplace_back(input_data[i].size());
    offset += input_data[i].size();
  }*/
  auto end = std::chrono::system_clock::now();
}

void initialization(vector<vector<int>> &hash_lsh,
                    std::vector<int> &a, std::vector<int> &lshnumber,
                    vector<tuple<int, int>> &rev_hash) {

  timer.start_time(init::init_data);
  //initialize_input_data(input_data, len_oristrings, idx_oristrings, oristrings);
  timer.end_time(init::init_data);
  setuplsh(hash_lsh, a, lshnumber, rev_hash);
}

void inititalize_dictory(uint8_t *dictory) {

  if (NUM_CHAR == 4) {
    dictory[static_cast<uint8_t>('A')] = 0;
    dictory[static_cast<uint8_t>('C')] = 1;
    dictory[static_cast<uint8_t>('G')] = 2;
    dictory[static_cast<uint8_t>('T')] = 3;
  } else if (NUM_CHAR == 5) {
    dictory[static_cast<uint8_t>('A')] = 0;
    dictory[static_cast<uint8_t>('C')] = 1;
    dictory[static_cast<uint8_t>('G')] = 2;
    dictory[static_cast<uint8_t>('T')] = 3;
    dictory[static_cast<uint8_t>('N')] = 4;
  } else if (NUM_CHAR == 26 || NUM_CHAR == 25) {
    int j = 0;
    for (int i = (int)'A'; i <= (int)'Z'; i++) {
      dictory[i] = j;
      j++;
    }
  } else if (NUM_CHAR == 37) {
    int j = 0;
    for (int i = (int)'A'; i <= (int)'Z'; i++) {
      dictory[i] = j;
      j++;
    }
    for (int i = (int)'0'; i <= (int)'9'; i++) {
      dictory[i] = j;
      j++;
    }
    dictory[static_cast<uint8_t>(' ')] = j;
  } else {
    LOG(error)
        << "input error: check the dictory of your input\n";
    exit(-1);
  }
}

void allocate_work(vector<long> times, int num_dev, size_t units_to_allocate,
                   vector<vector<size_t>> &size_per_dev) {

  size_t idx_slowest = -1; // Id of slowest device
  size_t idx_fastest = -1; // Id of fastest device
  size_t n_fast = 0; // Number of batches to allocate to the fastest device
  size_t n_slow = 0; // Number of batches to allocate to the slowest device

  for (auto t : times) {
    LOG(debug) << "\tTimes kernel: " << (float)t / 1000 << "sec";
  }

  if (num_dev > 1) {
    // If there are 2 devices, compute the number of batches
    // to allocate to devices.
    // Note that at most 2 devices can be handled with this version of the
    // function

    long slowest = -1;
    long fastest = -1;
    if (times[0] <= 0 && times[1] <= 0) {
      slowest = 1;
      fastest = 1;
      idx_slowest = 0;
      idx_fastest = 1;
    } else if (times[0] <= 0 && times[1] > 0) {
      slowest = 1;
      idx_slowest = 1;
      fastest = 0;
      idx_fastest = 0;
    } else if (times[0] > 0 && times[1] <= 0) {
      slowest = 1;
      idx_slowest = 0;
      fastest = 0;
      idx_fastest = 1;
    } else {
      // Get the max and min time measured during profiling.
      // The max time is associated with the slowest device.
      // The min time is associated with the fastest device.
      // Get the position in the time vector corresponding
      // to the min and max time.
      // These positions correspond to the device positions
      // in the vector containing device queues.

      auto max_iter = std::max_element(times.begin(), times.end());
      slowest = *max_iter;
      idx_slowest = max_iter - times.begin();

      idx_fastest = 1 - idx_slowest;
      fastest = times[idx_fastest];
    }
    n_slow = floor(((float)fastest / (float)(fastest + slowest)) *
                   units_to_allocate);
    n_fast = units_to_allocate - n_slow;

    size_per_dev[idx_fastest].emplace_back(n_fast);
    size_per_dev[idx_slowest].emplace_back(n_slow);
  } else if (num_dev == 1) {
    // If there is only one device, all remaining batches
    // are given to the first (and only) device of the queue.

    idx_fastest = 0;
    idx_slowest = -1;
    vector<size_t> tmp_sizes;
    n_slow = 0;
    n_fast = units_to_allocate;
    size_per_dev[idx_fastest].emplace_back(n_fast);
  }
  LOG(debug) << "\tn_fast: " << n_fast;
  LOG(debug) << "\tn_slow: " << n_slow;

  LOG(debug) << "\tid_fastest: " << idx_fastest;
  LOG(debug) << "\tid_slowest: " << idx_slowest << std::endl;

  int n = 0;
  for (auto d : size_per_dev) {
    LOG(debug) << "\tDev " << n << ":";
    int i = 0;
    for (auto s : d) {
      LOG(debug) << "\t\t" << i << ". " << s << std::endl;
      i++;
    }
  }
}

void split_buffers(vector<vector<size_t>> &size_per_dev, size_t size_element,
                   size_t limit = max_buffer_size) {

  int num_dev = size_per_dev.size();
  size_t tmp_size = 0;

  if (num_dev > 0) {
    for (int d = 0; d < num_dev; d++) {
      if (size_per_dev[d].size() != 1) {
        LOG(error)
            << "Only one element should be in the vector at this point"
            << std::endl;
        exit(-1);
      }
      size_t size = size_per_dev[d][0];
      size_t num_part = 1;
      while (size * size_element / num_part > limit) {
        num_part++;
      }
      // num_part++;
      LOG(debug) << "\tSplit buffer in " << num_part
                               << " parts of " << size / num_part << " as dim.";
      size_per_dev[d].clear();
      for (int j = 0; j < num_part; j++) {
        if (j == num_part - 1) {
          size_per_dev[d].emplace_back(size / num_part + size % num_part);
        } else {
          size_per_dev[d].emplace_back(size / num_part);
        }
      }
    }
  }
}

void create_buckets_wrapper(vector<char> &embdata,
                            vector<buckets_t> &buckets,
                            size_t num_strings,
                            vector<int> &a,
                            vector<int> &lshnumber,
                            size_t len_output) {

    LOG(LoggerLevel::info) << "Create buckets" << std::endl;
    LOG(debug) << "\tLen output: " << len_output;

    vector<long> times;

    vector<size_t> split_size;
    uint8_t dictory[256] = {0};

    inititalize_dictory(dictory);

    tbb::parallel_for(tbb::blocked_range2d<size_t>(0, num_strings * NUM_STR * NUM_REP, 0, NUM_HASH),
                      [&]( const tbb::blocked_range2d<size_t> &rangeIdx ){
      for(size_t index0 = rangeIdx.rows().begin(), index0_end = rangeIdx.rows().end(); index0 < index0_end; index0++) {
          for (size_t index1 = rangeIdx.cols().begin(), index1_end = rangeIdx.cols().end();
               index1 < index1_end; index1++) {

              size_t itq = index0;
              size_t i = itq / (NUM_STR * NUM_REP);// + acc_split_offset[0];
              size_t tq = itq % (NUM_STR * NUM_REP);
              size_t t = tq / NUM_REP;
              size_t q = tq % NUM_REP;

              size_t k = index1;//[1];

              size_t id = 0;
              char dict_index = 0;
              uint32_t id_mod = 0;
              size_t digit = -1;

              for (uint32_t j = 0; j < NUM_BITS; j++) {
                  digit = k * NUM_BITS + j;
                  dict_index = embdata[ABSPOS(i, t, q, digit, len_output)];
                  id += (dictory[dict_index]) * a[j];
              }

              id_mod = id % HASH_SZ;
              size_t output_position = index0 * rangeIdx.cols().size() + index1;

              buckets[output_position].idx_rand_str = t;
              buckets[output_position].idx_hash_func = k;
              buckets[output_position].hash_id = id_mod;
              buckets[output_position].idx_str = i;
              buckets[output_position].idx_rep = q;
          }
      }
    });
  timer.end_time(buckets::compute); // Start actual computing
}




void generate_random_string(int *p, int len_p) {

  for (int j = 0; j < NUM_STR; j++) {
    for (int t = 0; t < NUM_CHAR; t++) {
      for (int d = 0; d < samplingrange + 1; d++) {
        p[ABSPOS_P(j, t, d, len_p)] = 1 - rand() % 2;
      }
      for (int d = 0; d < samplingrange + 1; d++) {
        if (p[ABSPOS_P(j, t, d, len_p)] == 1) {
          if (d > 0 && p[ABSPOS_P(j, t, d - 1, len_p)] == 1) {
            p[ABSPOS_P(j, t, d, len_p)] = p[ABSPOS_P(j, t, d - 1, len_p)] - 1;
          } else {
            int next = d + 1;
            while (next < samplingrange + 1 &&
                   p[ABSPOS_P(j, t, next, len_p)] == 1) {
              p[ABSPOS_P(j, t, d, len_p)]++;
              next++;
            }
          }
        }
      }
    }
  }
}

void parallel_embedding_wrapper(std::vector<uint32_t> &len_oristrings,
                                std::vector<size_t> &idx_oristrings,
                                std::vector<char> &oristrings,
                                vector<char> &embdata,
                                size_t num_strings,
                                std::vector<int> &lshnumber, size_t &len_output,
                                std::vector<tuple<int, int>> &rev_hash) {

  LOG(LoggerLevel::info) << "Parallel Embedding" << std::endl;
  /**
   * Initialize the "dictory" that contains the translation
   * character -> number
   * */
  uint8_t dictory[256] = {0};
  inititalize_dictory(dictory);

  LOG(debug) << "\tLen output: " << len_output << std::endl;
  timer.start_time(embed::rand_str);
  /**
   * Allocate and initialize random strings to use for embedding
   * **/
  uint32_t len_p = samplingrange + 1;
  int *p = new int[NUM_STR * NUM_CHAR * len_p];
  generate_random_string(p, len_p);
  timer.end_time(embed::rand_str);


  tbb::parallel_for( tbb::blocked_range3d<size_t>(0, idx_oristrings.size(), 0, NUM_STR, 0, NUM_REP),
      [&]( const tbb::blocked_range3d<size_t> &rangeIdx ) {
       for(size_t id = rangeIdx.pages().begin(), id_end = rangeIdx.pages().end(); id < id_end; id++){
           for(size_t l = rangeIdx.rows().begin(), l_end = rangeIdx.rows().end(); l < l_end; l++){
               for(size_t k = rangeIdx.cols().begin(), k_end = rangeIdx.cols().end(); k<k_end; k++){
                   int partdigit = 0;
                   size_t size = len_oristrings[id];
                   size_t start_idx = idx_oristrings[id];
                   int r = 0;
                   int len_out = lshnumber.size();
                   int i = SHIFT * k;
                   int len = samplingrange + 1;

                   for (int j = 0; i < size && j <= samplingrange; i++) {
                       char s = oristrings[start_idx + i];
                       r = dictory[s];
                       j += (p[ABSPOS_P(l, r, j, len)] + 1);

                       while (partdigit < len_out && j > lshnumber[partdigit]) {

                           embdata[ABSPOS(id, l, k, std::get<0>(rev_hash[partdigit]),
                                          len_output)] = s;
                           int next = std::get<1>(rev_hash[partdigit]);

                           while (next != -1) {
                               embdata[ABSPOS(id, l, k, std::get<0>(rev_hash[next]),
                                              len_output)] = s;
                               next = get<1>(rev_hash[next]);
                           }
                           partdigit++;
                       }
                   }
               }
           }
       }
  });
  delete[] p;
}



bool onejoin(InputData &input_data, vector<buckets_t> &buckets, size_t offset_val, uint32_t new_samplingrange,
                       Time &t,
                       OutputValues &output_vals, string dataset_name) {

  timer=t;
  timer.start_time(total_alg::total);
  samplingrange = new_samplingrange;

  offset = offset_val;

  size_t len_output = NUM_HASH * NUM_BITS;

  size_t tot_input_size = input_data.oristrings.size();//0;

  size_t num_strings = input_data.N;

  print_configuration(len_output, num_strings,
                      samplingrange);


  LOG(LoggerLevel::info) << "Total chars read: "<<tot_input_size;

  // VARIABLES:
  /* HASH
   * a: the random vector for second level hash table
   * lshnumber: all distinct lsh bits actually used
   * hash_lsh: matrix of hash functions and their bits
   * rev_hash: vector containing the position of lsh bits into an embedded
   * string
   */
  vector<int> a;
  vector<int> lshnumber;
  vector<vector<int>> hash_lsh(NUM_HASH, vector<int>(NUM_BITS));
  vector<tuple<int, int>> rev_hash;

  /**
   * INPUT:
   * oristrings: array of rows, containing the input strings to use inside the
   * embed kernel len_oristrings: actual len of each input string
   * embedded_data: embedded dataset
   * */

  std::vector<char> embedded_data;

  /**
   * OUTPUT:
   * output_pairs: contains the IDs of strings in the input vector
   * 				 that are similar according edit distance
   * */
  vector<idpair> output_pairs;

  /*
   * OTHER:
   * buckets: vector containing lsh buckets;
   * 			the first 3 elements identify the buckets;
   * 			the 4th and 5th element are respectively
   * 			the input string id and the replication id.
   *
   * candidates:  vector containing the candidates pairs of strings
   * 				that will be processed and verified by means of
   * 				edit distance computaton;
   * 				the 1st and 3rd element contain the id of input
   *				strings of a candidate pair; the 2nd element contains the
   *difference of lenghts of 2 strings the 4th element in an uint32_t  type and
   *contains use 3 bits to contain the replication id of first string, 3 bit for
   *the replication id of second string, and 1 bits that say if the 2 strings
   *have all lsh bits equal.
   *
   **/
  vector<candidate_t> candidates;

  if(NUM_REP>127){
    LOG(error)
        << "Are not supported more than 127 sub-strings (ED_DIST/SHIFT) per input string.";
    exit(-1);
  }

  /**
   * INITIALIZATION
   * */

  timer.start_time(init::total);
  srand(11110);
  initialization(hash_lsh, a, lshnumber, rev_hash);
  timer.end_time(init::total);
  LOG(LoggerLevel::info) << "Start parallel algorithm..." << std::endl;

  /**
   * EMBEDDING STEP
   **/

  timer.start_time(embed::total);

  timer.start_time(embed::alloc);
  
  //embedded_data.resize(n_batches);

  embedded_data.resize(num_strings * NUM_STR * NUM_REP * len_output, 0);

  timer.end_time(embed::alloc);

  parallel_embedding_wrapper(input_data.len_oristrings, input_data.idx_oristrings, input_data.oristrings,
		  embedded_data, num_strings, lshnumber, len_output, rev_hash);

  timer.end_time(embed::total);

  timer.start_time(lsh::total);

  /**
   * CREATE BUCKETS STEP
   ***/
  timer.start_time(buckets::total);

  timer.start_time(buckets::allocation);

  try {
    buckets.resize(num_strings * NUM_STR * NUM_HASH * NUM_REP);
  } catch (std::bad_alloc &e) {
    LOG(error)
        << "It is not possible allocate the requested size.";
    exit(-1);
  }
  timer.end_time(buckets::allocation);

  create_buckets_wrapper(embedded_data, buckets, num_strings, a, lshnumber, len_output);

  timer.start_time(buckets::sort);
  tbb::parallel_sort(buckets.begin(), buckets.end());
  timer.end_time(buckets::sort);

  timer.end_time(buckets::total);

  timer.end_time(total_alg::total);

  t = timer;

  return true;
}
