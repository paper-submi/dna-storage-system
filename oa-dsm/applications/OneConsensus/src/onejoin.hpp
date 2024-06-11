#ifndef ONEJOIN_H
#define ONEJOIN_H
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <tuple>
#include <cmath>
#include <cstdint>
#include <unistd.h>

#include "tbb/parallel_sort.h"
#include "Time.hpp"
#include "constants.hpp"
#include "utils.hpp"


using idpair=std::tuple<int, int>;


struct batch_hdr{
	size_t size;
	size_t offset;
	batch_hdr(size_t size, size_t offset): size(size), offset(offset){}
};


int edit_distance(const char *x, const int x_len, const  char *y, const int y_len, int k);

bool onejoin(InputData &input_data,
	 std::vector<buckets_t> &buckets, size_t offset_val,
	 uint32_t new_samplingrange,
	  Time &timer, OutputValues &output_val, std::string dataset_name="");

#endif
