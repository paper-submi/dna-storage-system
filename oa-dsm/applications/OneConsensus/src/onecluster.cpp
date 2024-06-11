#ifndef DBSCAN_H
#define DBSCAN_H

#include <mutex>
#include <algorithm>
#include <numeric>
#include "../../../src/AlignmentTools/Aligner.h"
#include "../../../src/Common/Logger.h"

#include "onecluster.hpp"
#include "../../../src/AlignmentTools/edlib/include/edlib.h"

using namespace std;
using namespace constants;

//const int PRIMER_LENGTH=20;
const int NUM_PTS=1;
const int32_t NTS=16;

struct ProbabilityVector{
    vector<int32_t> probs;
    friend ofstream& operator<<(ofstream& out, ProbabilityVector& probability){
        for(auto&f:probability.probs){
            out<<f<<" ";
        }
        return out;
    }
};
void clustering_short_reads( InputData &input_data, size_t right_length, std::vector<std::vector<std::string>>& consensus_results, std::vector<std::vector<uint32_t>>& consensus_results_pts, vector<vector<ProbabilityVector>>& probability, std::vector<std::vector<uint32_t>>& buckets_map );
void clustering_long_reads( InputData &input_data, size_t right_length, std::vector<std::vector<std::string>>& consensus_results, std::vector<std::vector<uint32_t>>& consensus_results_pts, vector<vector<ProbabilityVector>>& probability, std::vector<std::vector<uint32_t>>& buckets_map );

void printClusters(vector<uint32_t> &input_cluster, InputData &input_dataset, ofstream& output_file){
    output_file << input_cluster.size() << std::endl;
    for( auto& strIdx : input_cluster ){
        output_file << input_dataset( strIdx ) << std::endl;
    }
}

void reorder(vector<uint32_t>& vA, vector<size_t>& vI)
{
    size_t i, j, k;
    size_t t;
    for(i = 0; i < vA.size(); i++){
        if(i != vI[i]){
            t = vA[i];
            k = i;
            while(i != (j = vI[k])){
                // every move places a value in it's final location
                vA[k] = vA[j];
                vI[k] = k;
                k = j;
            }
            vA[k] = t;
            vI[k] = k;
        }
    }
}

uint32_t process_cluster(vector<uint32_t> &input_cluster, InputData &input_dataset, string &true_string, vector<uint32_t> &counter, vector<vector<kmer_t>>& kmers, vector<AlignResult>& cigars, size_t right_lenght, ofstream& output_file){

    std::unordered_map<kmer_t, uint16_t> kmer_map;

    for(auto& kmer : kmers){
        for(auto& k : kmer){
            if( kmer_map.count(k) ){
                kmer_map[k]++;
            }else{
                kmer_map[k]=1;
            }
        }
    }

    vector<uint32_t> read_score(input_cluster.size(), 0);

    for(size_t i=0; i<input_cluster.size(); i++) {
        for (auto &k: kmers[i]) {
            read_score[i]+=kmer_map[k];
        }
    }
    kmer_map.clear();

    vector<size_t> indices(input_cluster.size());

    std::iota(indices.begin(), indices.end(), 0);

    sort(indices.begin(), indices.end(), [&](size_t e1, size_t e2){
        return read_score[e1]>read_score[e2];
    });

    reorder(input_cluster, indices);
    indices.clear();

    return 0;
}

void organize_buckets( std::vector<buckets_t>& buckets, std::vector<std::vector<uint32_t>>& buckets_map ) {
    buckets_map.emplace_back(vector<uint32_t>{});

    for(int i=0; i<buckets.size()-1; i++){

        buckets_map.back().emplace_back(buckets[i].idx_str);

        if( (buckets[i].idx_rand_str != buckets[i + 1].idx_rand_str) ||
            (buckets[i].idx_rand_str == buckets[i + 1].idx_rand_str &&
             buckets[i].idx_hash_func != buckets[i + 1].idx_hash_func) ||
            (buckets[i].idx_rand_str == buckets[i + 1].idx_rand_str &&
             buckets[i].idx_hash_func == buckets[i + 1].idx_hash_func &&
             buckets[i].hash_id != buckets[i + 1].hash_id) ){

            buckets_map.emplace_back(vector<uint32_t>{});

        }
    }
    buckets_map.back().emplace_back(buckets.back().idx_str); // The last one
}

void one_consensus(InputData &input_data, size_t right_length, size_t offset, uint32_t new_samplingrange, Time &timer, string dataset_name) {

    timer.start_time(cluster::total);

    std::vector<buckets_t> buckets;
    OutputValues ov;

    vector<vector<uint32_t>> buckets_map;

    timer.start_time(cluster::onejoin);

    onejoin( input_data, buckets, offset, new_samplingrange, timer, ov, dataset_name);

    timer.end_time(cluster::onejoin);

    timer.start_time(cluster::create_indexes);

    organize_buckets(buckets, buckets_map);

    LOG(info)<<"Number of buckets: "<<buckets_map.size()<<std::endl;
    buckets.clear();

    timer.end_time(cluster::create_indexes);

    timer.start_time(cluster::dbscan);

#ifdef SHORT_READ

    LOG(info)<<"Reordering string within buckets";

    tbb::parallel_for(static_cast<size_t>(0), static_cast<size_t>(buckets_map.size()), [&](size_t t_idx){
        std::partition(buckets_map[t_idx].begin(), buckets_map[t_idx].end(), [&](uint32_t& i1){
            return input_data.len_oristrings[i1]==right_length;
        });
    });
#endif
    vector<vector<string>> consensus_results(12, vector<string>{});
    vector<vector<uint32_t>> consensus_results_pts(12, vector<uint32_t>{});
    vector<vector<ProbabilityVector>> probability(12, vector<ProbabilityVector>{});

#ifdef SHORT_READ
    LOG(info)<<"Computing clustering and consensus short reads";
    clustering_short_reads( input_data, right_length, consensus_results, consensus_results_pts, probability, buckets_map );
#endif

#ifdef LONG_READ
    LOG(info)<<"Computing clustering and consensus long reads";
    clustering_long_reads( input_data, right_length, consensus_results, consensus_results_pts, probability, buckets_map );
#endif

    cout<<std::endl;
    LOG(info)<<"COMPLETED!";

    timer.end_time(cluster::dbscan);

    timer.end_time(cluster::total);
}

void clustering_short_reads( InputData &input_data, size_t right_length, std::vector<std::vector<std::string>>& consensus_results, std::vector<std::vector<uint32_t>>& consensus_results_pts, vector<vector<ProbabilityVector>>& probability, std::vector<std::vector<uint32_t>>& buckets_map){

    std::atomic<int> bucket_idx(0);
    size_t size_buckets_map = buckets_map.size();

    tbb::parallel_for(static_cast<size_t>(0), static_cast<size_t>(12), [&](size_t thread_idx) {

        size_t num_buckets_processed=0;

        ofstream clusters_thread("clusters_thread_"+std::to_string(thread_idx));

#ifdef  DEBUG
        ofstream clusters_thread_debug("debug_clusters_thread_"+std::to_string(thread_idx));
#else
        ofstream clusters_thread_debug;
#endif

        vector <uint32_t> counter(256, 0);
        vector <uint32_t> clusters;
        vector <AlignResult> cigars;

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
                    // There is only one point
                    clusters_thread << "1" << std::endl;
                    clusters_thread << input_data.to_string(b[0]) << std::endl;
                }

                for (int i = 0; i < b.size() - 1; i++) {

                    uint32_t id_1 = b[i];

                    if (!processed[i]) {

                        processed[i] = true;

                        clusters.emplace_back(id_1);

                        for (int j = i + 1; j < b.size(); j++) {

                            if (!processed[j]) {

                                uint32_t id_2 = b[j];
                                AlignResult ez;
                                std::string s1 = input_data.to_string(id_1);
                                std::string s2 = input_data.to_string(id_2);
                                auto ed = Aligner::EditDistance(s1, s2, K_INPUT);
                                if (ed >= 0) {
                                    processed[j] = true;
                                    clusters.emplace_back(id_2);
                                }
                            }
                        }
                        printClusters(clusters, input_data, clusters_thread);

                        clusters.clear();
                        cigars.clear();
                    }
                }
            } else {
                break;
            }
//            if( (num_buckets_processed%10000)==0 ){
//                log_thread<<num_buckets_processed<<std::endl;
//            }
        }
    });
}

void clustering_long_reads( InputData &input_data, size_t right_length, std::vector<std::vector<std::string>>& consensus_results, std::vector<std::vector<uint32_t>>& consensus_results_pts, vector<vector<ProbabilityVector>>& probability, std::vector<std::vector<uint32_t>>& buckets_map){

    std::atomic<int> bucket_idx(0);
    size_t size_buckets_map = buckets_map.size();

    size_t K=5;

    tbb::parallel_for(static_cast<size_t>(0), static_cast<size_t>(12), [&](size_t thread_idx){

        size_t num_buckets_processed=0;

        ofstream clusters_thread("clusters_thread_"+std::to_string(thread_idx));
        //ofstream log_thread("log_thread_"+std::to_string(thread_idx));
#ifdef DEBUG
        ofstream clusters_thread_debug("debug_clusters_thread_"+std::to_string(thread_idx));
#else
        ofstream clusters_thread_debug;
#endif

        vector<uint32_t> counter(256,0);
        vector<uint32_t> clusters;
        vector<AlignResult> cigars;
        vector<vector<kmer_t>> kmers;

        while(true){
            size_t to_process = bucket_idx.fetch_add(1);
            num_buckets_processed++;
            if(to_process<size_buckets_map){

                vector<uint32_t> b = buckets_map[to_process];

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

                if( processed.size()==1 ) {
                    // There is only one point
                    if( NUM_PTS==1 ){
                        //consensus_results[thread_idx].emplace_back(input_data.to_string(b[0]));
                        //consensus_results_pts[thread_idx].emplace_back((uint32_t)1);
                        clusters_thread<<"1"/*<<" "<<input_data.to_string(b[0])<< " 0" */<< std::endl;
                        clusters_thread<<input_data.to_string(b[0])<<std::endl;
                    }
                }

                for(int i=0; i<b.size()-1; i++){

                    uint32_t id_1 = b[i];

                    if( !processed[i] ){

                        processed[i] = true;
                        clusters.emplace_back(id_1);

                        string_view s1(input_data[id_1].operator->(), input_data.len_oristrings[id_1]);

                        kmers.emplace_back(vector<kmer_t>{});

                        get_kmers(kmers.back(), s1, K);

                        for (int j = i + 1; j < b.size(); j++) {

                            if( !processed[j] ) {

                                uint32_t id_2 = b[j];

                                AlignResult ez;
                                string_view s2(input_data[id_2].operator->(), input_data.len_oristrings[id_2]);

                                vector<kmer_t> kmers_s2;

                                get_kmers(kmers_s2, s2, K);

                                if( !kmers_s2.empty() ) {
                                    sort(kmers_s2.begin(), kmers_s2.end());
                                    if (kmers_simil(kmers.front(), kmers_s2, (right_length - K + 1) * 0.5 )) {
                                        processed[j] = true;
                                        clusters.emplace_back(id_2);
                                        kmers.emplace_back(kmers_s2);
                                    }
                                }

                            }
                        }
                        printClusters(clusters, input_data, clusters_thread);
                        //string true_string="";
                        //size_t score = process_cluster(clusters, input_data, true_string, counter, kmers, cigars, right_length, clusters_thread);
                        //ProbabilityVector local_probability;

                        //get_consensus(clusters, input_data, true_string, counter, cigars, local_probability, clusters_thread, clusters_thread_debug);

                        /*if(!true_string.empty()){
                            consensus_results[thread_idx].emplace_back(true_string);
                            consensus_results_pts[thread_idx].emplace_back(clusters.size());
                            probability[thread_idx].emplace_back(local_probability);
                        }*/
                        clusters.clear();
                        cigars.clear();
                        kmers.clear();
                    }
                }
            }
            else{
                break;
            }
//            if( (num_buckets_processed%10000)==0 ){
//                log_thread<<num_buckets_processed<<std::endl;
//            }
        }
    });
}

#endif
