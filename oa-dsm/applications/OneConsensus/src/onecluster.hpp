

#ifndef SRC_ONECLUSTER_HPP_
#define SRC_ONECLUSTER_HPP_

#include <vector>
#include <set>
#include <unordered_map>
#include <cstdint>

#include "Time.hpp"
#include "utils.hpp"
#include "onejoin.hpp"

constexpr int UNDEFINED=-2;
constexpr int NOISE=-1;

using EmbeddedSet = std::vector<std::vector<char>>;

void one_consensus(InputData &input_data, size_t right_length, size_t offset, uint32_t new_samplingrange, Time &timer, std::string dataset_name);

#endif /* SRC_ONECLUSTER_HPP_ */
