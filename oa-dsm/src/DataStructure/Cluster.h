#ifndef CONSENSUS_CLUSTER_H
#define CONSENSUS_CLUSTER_H

#include <vector>
#include <string>
#include <string_view>
#include "Motif.h"

class index_not_available : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Index not available";
    }
};

class invalid_cluster_file : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Cannot open cluster file.";
    }
};

class invalid_cluster_format : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Cannot open cluster file.";
    }
};

class invalid_consensus_argument : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Index not available";
    }
};

class ConsensusResult{
    std::string motif;
    float llrCoefficient;
public:
    ConsensusResult(std::string_view oligo, float coef) : motif(oligo), llrCoefficient(coef){}
    [[nodiscard]] float GetCoefficientLLR() const;
    std::string GetMotif();
};


class Cluster {
    std::vector<std::string> originalRead;
public:
    void AddRead(std::string_view read) {
        originalRead.emplace_back(read);
    }
    const std::string& GetReadAt(std::uint64_t idx) {
        return originalRead[idx];
    }
    const std::vector<std::string>& GetAllReads(){
        return originalRead;
    }
};


class ClusterView {
    std::vector<std::string_view> reads;

    std::int32_t ComputeScore(std::string_view resultingMotif, std::uint32_t szMotif);
    static void TrimReads(std::string_view correctMotif, std::vector<std::string_view>& motif, std::uint32_t ALN_WINDOW= 4);

public:

    explicit ClusterView() = default;

    ConsensusResult GetConsensus(std::uint32_t szMotif, std::uint32_t AlignmentWindow = 4);
    void AdjustReads(std::string_view correctMotif, std::uint32_t ALN_WINDOW = 4);

    std::string_view GetReadAt(std::uint64_t idx);

    void SetReadAt(std::uint64_t idx, std::string_view read);
    void AddRead(std::string_view read);

    size_t Size();
    void Clear();

    bool CheckPointersValidity();
};

class ClustersDataset {
    std::vector<Cluster> dataset;
public:
    void AddCluster(Cluster& cluster);
    ClusterView GetView(std::uint64_t idx);
    std::uint64_t Size();
    static void ImportClusters(std::string_view filename, ClustersDataset& dataset, std::uint32_t MotifLen);
};

#endif //CONSENSUS_CLUSTER_H
