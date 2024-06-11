//
// Created by Eugenio Marinelli on 12/01/2022.
//

#ifndef ONECONSENSUS_FUNCTIONS_H
#define ONECONSENSUS_FUNCTIONS_H

#include <string>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <cstdint>

#include "edlib/include/edlib.h"
#include "Time.hpp"
#include "utils.hpp"
#include "onejoin.hpp"


void clustering_short_reads(InputData &input_data, size_t rigth_length, std::vector<std::vector<std::string>>& consensus_results, std::vector<std::vector<uint32_t>>& consensus_results_pts, std::vector<std::vector<uint32_t>>& buckets_map);
void clustering_long_reads( InputData &input_data, size_t rigth_length, std::vector<std::vector<std::string>>& consensus_results, std::vector<std::vector<uint32_t>>& consensus_results_pts, std::vector<std::vector<uint32_t>>& buckets_map);

#endif //ONECONSENSUS_FUNCTIONS_H
