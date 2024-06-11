#include "functions.h"


using namespace std;

void clustering_short_reads( InputData &input_data, size_t right_length, std::vector<std::vector<std::string>>& consensus_results, std::vector<std::vector<uint32_t>>& consensus_results_pts, std::vector<std::vector<uint32_t>>& buckets_map){

    std::atomic<int>& bucket_idx(0);
    size_t size_buckets_map = buckets_map.size();

    tbb::parallel_for(static_cast<size_t>(0), static_cast<size_t>(12), [&](size_t thread_idx) {
        ofstream clusters_thread("clusters_thread_" + std::to_string(thread_idx));

        vector <uint32_t> counter(256, 0);
        vector <uint32_t> clusters;
        vector <string> cigars;

        while (true) {
            int to_process = bucket_idx.fetch_add(1);

            if (to_process < size_buckets_map) {

                vector <uint32_t> b = buckets_map[to_process];

                vector<bool> processed(b.size(), false);

                /* 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
                * 0  0  0  0  0  0  0  0  0   0
                * 1,2 -> N
                * 1,3 -> Y
                * 1,4 -> Y
                * ...
                * 1  0  1  1  1  0  0  0  0   0
                * 2,6
                */

                if (processed.size() == 1) {
                    //single_pts_clusters++;
                    // There is only one point
                    if (NUM_PTS == 1 && input_data.to_string(b[0]).size() == right_length) {
                        consensus_results[thread_idx].emplace_back(input_data.to_string(b[0]));
                        consensus_results_pts[thread_idx].emplace_back((uint32_t) 1);
                        clusters_thread << "1" << " " << input_data.to_string(b[0]) << std::endl;
                        clusters_thread << input_data.to_string(b[0]) << std::endl;
                    }
                }

                for (int i = 0; i < b.size() - 1; i++) {

                    uint32_t id_1 = b[i];

                    if (!processed[i]) {

                        processed[i] = true;

                        clusters.emplace_back(id_1);
                        cigars.emplace_back(string{});

                        for (int j = i + 1; j < b.size(); j++) {

                            if (!processed[j]) {

                                uint32_t id_2 = b[j];

                                std::string ez;
                                int ed = get_cigar(input_data, id_1, id_2, ez);

                                if (ed >= 0) {
                                    processed[j] = true;
                                    clusters.emplace_back(id_2);
                                    cigars.emplace_back(ez);
                                }
                            }
                        }

                        string true_string = "";
                        /*
                         * If the first read in the cluster has a length that is different from the
                         * right length of the references, the centroids will have the wrong length
                         * and it will be discharde. Thus, let's avoid to compute the centroid for
                         * that cluster.
                         * */
                        if (input_data.to_string(clusters[0]).size() == right_length) {
                            get_consensus(clusters, input_data, true_string, counter, cigars, clusters_thread);
                        }
                        if (true_string != "") {
                            consensus_results[thread_idx].emplace_back(true_string);
                            consensus_results_pts[thread_idx].emplace_back(clusters.size());
                        }
                        clusters.clear();
                        cigars.clear();
                    }
                }
            } else {
                break;
            }
        }
    });
}

void clustering_long_reads( InputData &input_data, size_t right_length, std::vector<std::vector<std::string>>& consensus_results, std::vector<std::vector<uint32_t>>& consensus_results_pts, std::vector<std::vector<uint32_t>>& buckets_map){

    std::atomic<int>& bucket_idx(0);
    size_t size_buckets_map = buckets_map.size();

    tbb::parallel_for(static_cast<size_t>(0), static_cast<size_t>(12), [&](size_t thread_idx) {
        ofstream clusters_thread("clusters_thread_" + std::to_string(thread_idx));

        vector <uint32_t> counter(256, 0);
        vector <uint32_t> clusters;
        vector <string> cigars;

        while (true) {
            int to_process = bucket_idx.fetch_add(1);

            if (to_process < size_buckets_map) {

                vector <uint32_t> b = buckets_map[to_process];

                vector<bool> processed(b.size(), false);

                /* 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
                * 0  0  0  0  0  0  0  0  0   0
                * 1,2 -> N
                * 1,3 -> Y
                * 1,4 -> Y
                * ...
                * 1  0  1  1  1  0  0  0  0   0
                * 2,6
                */

                if (processed.size() == 1) {
                    //single_pts_clusters++;
                    // There is only one point
                    if (NUM_PTS == 1 && input_data.to_string(b[0]).size() == right_length) {
                        consensus_results[thread_idx].emplace_back(input_data.to_string(b[0]));
                        consensus_results_pts[thread_idx].emplace_back((uint32_t) 1);
                        clusters_thread << "1" << " " << input_data.to_string(b[0]) << std::endl;
                        clusters_thread << input_data.to_string(b[0]) << std::endl;
                    }
                }

                for (int i = 0; i < b.size() - 1; i++) {

                    uint32_t id_1 = b[i];

                    if (!processed[i]) {

                        processed[i] = true;

                        clusters.emplace_back(id_1);
                        cigars.emplace_back(string{});

                        for (int j = i + 1; j < b.size(); j++) {

                            if (!processed[j]) {

                                uint32_t id_2 = b[j];

                                std::string ez;
                                int ed = get_cigar(input_data, id_1, id_2, ez);

                                if (ed >= 0) {
                                    processed[j] = true;
                                    clusters.emplace_back(id_2);
                                    cigars.emplace_back(ez);
                                }
                            }
                        }

                        string true_string = "";
                        /*
                         * If the first read in the cluster has a length that is different from the
                         * right length of the references, the centroids will have the wrong length
                         * and it will be discharde. Thus, let's avoid to compute the centroid for
                         * that cluster.
                         * */
                        if (input_data.to_string(clusters[0]).size() == right_length) {
                            get_consensus(clusters, input_data, true_string, counter, cigars, clusters_thread);
                        }
                        if (true_string != "") {
                            consensus_results[thread_idx].emplace_back(true_string);
                            consensus_results_pts[thread_idx].emplace_back(clusters.size());
                        }
                        clusters.clear();
                        cigars.clear();
                    }
                }
            } else {
                break;
            }
        }
    });
}