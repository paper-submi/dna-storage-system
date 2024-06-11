
#include <fstream>
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include "Cluster.h"
#include "../AlignmentTools/Aligner.h"

enum InputFileFormat {
    fasta,
    cluster,
    unknown
};

struct MajorityResult{
    std::string motif;
    std::int32_t score;
};

MajorityResult GetMajority(std::unordered_map<std::string, std::int32_t>& motifMap){
    if (!motifMap.empty()) {
        auto max = std::max_element(motifMap.begin(), motifMap.end(), [](const auto &p1, const auto &p2) {
            return p1.second < p2.second;
        });
        return {  max->first, max->second };
    }
    return { "", -1};
}

ConsensusResult ClusterView::GetConsensus(std::uint32_t szMotif, std::uint32_t AlignmentWindow){
    if ((szMotif % AlignmentWindow ) != 0) {
        throw invalid_consensus_argument();
    }

    if (this->reads.empty()) {
        return {"", 0};
    }

    // Make a temporary copy of reads in the cluster, until the 2nd motif.
    // The copy it's necessary because afterward, we need to compute the edit distance
    // considering the whole motif, instead of the sub-motif.
    std::vector<std::string_view> motifs(this->reads.size(), "");

    // Get until 2nd motif if possible
    for(size_t i=0; i<this->reads.size(); i++){
        size_t n = (szMotif * 2 < this->reads[i].size()) ? (szMotif * 2) : std::string::npos;
        motifs[i] = this->reads[i].substr(0, n);
    }

    std::string resultingMotif;
    std::int32_t matches = 0;

    std::uint32_t NUMBER_OF_SUBMOTIFS = szMotif / AlignmentWindow;

    // Align one sub-motif at time
    for( size_t itr = 0; itr < NUMBER_OF_SUBMOTIFS; itr++ ) {
        std::unordered_map<std::string, std::int32_t> counter;
        for (size_t i = 0; i < motifs.size(); i++) {
            std::string_view motif = motifs[i];
            if (!motif.empty()) {
                std::string_view subMotif = motif.substr(0, AlignmentWindow);
                counter[std::string{subMotif}]++;
            }
        }

        MajorityResult majority = GetMajority(counter);
        if ( !majority.motif.empty() ) {
            resultingMotif += majority.motif;
        }
        // Trim the temporary motifs, based on the alignment of the first sub-motif
        // obtained by consensus with the original sub-motif
        ClusterView::TrimReads(majority.motif, motifs);
    }
    // Compute the score between the full original motif and the consensus result
    matches = ComputeScore(resultingMotif, szMotif);

    if ( !resultingMotif.empty() ) {
        if (resultingMotif.size() != szMotif) {
            size_t szFinal = (resultingMotif.size() + szMotif - 1) / szMotif * szMotif;
            size_t szInitial = resultingMotif.size();
            // Pad the last motif with A which is equivalent to pad with zeros
            size_t toPadd = szFinal - szInitial;
            for (size_t i = 0; i < toPadd; i++) {
                resultingMotif += 'A';
            }
            matches = 0;
        }
        return {resultingMotif, static_cast<float>(matches)};
    }
    return {resultingMotif, static_cast<float>(matches)};
}

std::int32_t ClusterView::ComputeScore(std::string_view resultingMotif, std::uint32_t szMotif){
    std::int32_t matches=0;

    for(size_t i=0; i<this->reads.size(); i++){
        size_t n = (szMotif < reads[i].size() ? szMotif : std::string::npos);
        auto readMotif = reads[i].substr(0, n);
        std::int32_t ED = Aligner::EditDistance(readMotif, resultingMotif);
        if(ED>=0 && ED<=2){
            matches++;
        }
    }
    return matches;
}

void ClusterView::AdjustReads(std::string_view correctMotif, std::uint32_t ALN_WINDOW ){
    if ((correctMotif.size() % ALN_WINDOW ) != 0) {
        throw invalid_consensus_argument();
    }
    std::uint32_t nIterations = correctMotif.size() / ALN_WINDOW;

    for (std::uint32_t itr = 0; itr<nIterations; itr++) {
        auto correctSubMotif = correctMotif.substr(itr*ALN_WINDOW, ALN_WINDOW);
        for (size_t r_id = 0; r_id < reads.size(); r_id++) {

            if (!reads[r_id].empty()) {
                auto end = ALN_WINDOW < reads[r_id].size() ? ALN_WINDOW : std::string::npos;//read.size()-SZ_MOTIF;
                std::string_view ntsN = reads[r_id].substr(0, end);

                if (correctSubMotif == ntsN) {
                    if (ALN_WINDOW < reads[r_id].size()) {
                        reads[r_id] = reads[r_id].substr(ALN_WINDOW, std::string::npos); //read.size()-SZ_MOTIF);
                    } else {
                        reads[r_id] = "";
                    }
                } else {
                    end = ((ALN_WINDOW + 3) < reads[r_id].size()) ? (ALN_WINDOW + 3) : std::string::npos;
                    std::string_view subread = reads[r_id].substr(0, end);

                    auto pos = Aligner::FindLocalAlignmentPositions(subread, correctSubMotif);
                    //std::int32_t ed = pos.editDistance;
                    if (/*ed<=3 &&*/ pos.end >= 0 && static_cast<size_t>(pos.end) < reads[r_id].size()) {
                        reads[r_id] = reads[r_id].substr(pos.end, std::string::npos);//read.size()-pos.second);
                    } else {
                        reads[r_id] = "";
                    }
                }
            }
        }
    }
}

void ClusterView::TrimReads(std::string_view correctMotif, std::vector<std::string_view>& motif, std::uint32_t ALN_WINDOW ){

    for (size_t r_id = 0; r_id < motif.size(); r_id++ ) {

        std::string_view read = motif[r_id];

        if (!read.empty()) {
            auto end = ALN_WINDOW < read.size() ? ALN_WINDOW : std::string::npos;//read.size()-SZ_MOTIF;
            std::string_view ntsN = read.substr(0, end);

            if (correctMotif == ntsN) {
                if ( ALN_WINDOW < read.size() ) {
                    motif[r_id] = read.substr(ALN_WINDOW, std::string::npos); //read.size()-SZ_MOTIF);
                } else {
                    motif[r_id] = "";
                }
            } else {
                end = ((ALN_WINDOW+3) < read.size()) ? (ALN_WINDOW+3) : std::string::npos;
                std::string_view readSelected = read.substr(0, end);

                auto pos = Aligner::FindLocalAlignmentPositions(readSelected, correctMotif);
                //std::int32_t ed=pos.editDistance;
                if (/*ed<=3 &&*/ pos.end >= 0 && static_cast<size_t>(pos.end) < read.size()) {
                    motif[r_id] = read.substr(pos.end, std::string::npos);//read.size()-pos.second);
                } else {
                    motif[r_id] = "";
                }
            }
        }
    }
}

void ClusterView::AddRead(std::string_view read){
    reads.emplace_back(read);
}

std::string_view ClusterView::GetReadAt(std::uint64_t idx){
    if (idx >= this->reads.size()) {
        throw index_not_available();
    }
    return reads[idx];
}

void ClusterView::SetReadAt(std::uint64_t idx, std::string_view read){
    if (idx > reads.size()) {
        throw index_not_available();
    }
    this->reads[idx] = read;
}

size_t ClusterView::Size() {
    return reads.size();
}

void ClusterView::Clear(){
    this->reads.clear();
}

bool ClusterView::CheckPointersValidity() {//TODO: Add test case
    bool all_valid = std::all_of(reads.begin(), reads.end(), [](std::string_view str){
        return !str.empty();
    });
    return all_valid;
}


std::string ConsensusResult::GetMotif(){
    return motif;
}

float ConsensusResult::GetCoefficientLLR() const {
    return this->llrCoefficient;
}

void ClustersDataset::ImportClusters(std::string_view filename, ClustersDataset& dataset, std::uint32_t MotifLen) {
    InputFileFormat format;
    {
        std::ifstream file(filename.data());
        if (!file.is_open()) {
            throw invalid_cluster_file();
        }
        std::string line;
        file >> line;
        if (line[0] == '>') {
            format = InputFileFormat::fasta;
        }else {
            try{
                stoll(line);
            }catch (std::exception& e){
                throw invalid_cluster_file();
            }
            format = InputFileFormat::cluster;
        }
    }
    std::ifstream file(filename.data());

    if(format == InputFileFormat::cluster) {
        std::string read;
        std::string header;
        std::int64_t N=0;
        //int i = 0;
        while (file >> header) {
            try {
                N = std::stoll(header);
            } catch (std::exception &e) {
                throw invalid_cluster_format();
            }
            if (N > 0) {
                Cluster cluster;
                for (ssize_t r_idx = 0; r_idx < N; r_idx++) {
                    file >> read;
                    size_t final_size = (read.size() + MotifLen - 1) / MotifLen * MotifLen;
                    size_t initial_size = read.size();
                    /* Pad the last motif with A which is equivalent to pad with zeros*/
                    for (size_t i = 0; i < final_size - initial_size; i++) {
                        read += 'A';
                    }
                    cluster.AddRead(read);
                }
                dataset.AddCluster(cluster);
            } else {
                throw invalid_cluster_format();
            }
        }
    } else if (format==InputFileFormat::fasta) {
        std::string refId;
        std::string read;
        while (file >> refId) {
            Cluster cluster;
            file >> read;
            size_t final_size = (read.size() + MotifLen - 1) / MotifLen * MotifLen;
            size_t initial_size = read.size();
            /* Pad the last motif with A which is equivalent to pad with zeros*/
            for (size_t i = 0; i < final_size - initial_size; i++) {
                read += 'A';
            }
            cluster.AddRead(read);
            dataset.AddCluster(cluster);
        }
    }
}

ClusterView ClustersDataset::GetView(std::uint64_t idx){
    ClusterView clusterView;
    for( const auto& read : dataset[idx].GetAllReads() ){
        clusterView.AddRead(read);
    }
    return clusterView;
}

void ClustersDataset::AddCluster(Cluster& cluster) {
    dataset.emplace_back(cluster);
}

std::uint64_t ClustersDataset::Size() {
    return dataset.size();
}