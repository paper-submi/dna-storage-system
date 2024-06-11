#ifndef PRIMERSREMOVAL_ALIGN_H
#define PRIMERSREMOVAL_ALIGN_H

#include <vector>
#include <string>
#include <string_view>

/** Unused */
//int32_t align_primers(std::string_view& read, std::string& primer3, std::string& primer5);
//void align_reads(std::string& target, std::string& read);

/**
 * Find the starting and ending positions of the motif alignment to the read.
 * */

/**
 * The following functions are used to find the primer who best aligns to the given read.
 * */
//bool keep_read_before(std::string_view &read_to_process, std::vector<std::string> &motifs, std::size_t &best_motif_idx, size_t MAX_ED);
//bool keep_read_after(std::string_view &read_to_process, std::vector<std::string> &motifs, std::size_t &best_motif_idx, size_t MAX_ED);

/**
 * Remove the primer, extracting only the payload of a give length
 * */
//bool remove_primer(std::string &read, std::string_view& result, std::vector<std::string> &primers, size_t right_length);

struct AlignmentPosition {
    std::int32_t start;
    std::int32_t end;
    std::int32_t editDistance;
};

class Aligner {
    static AlignmentPosition GetLocalAlignment(std::string_view oligo, std::string_view motif, std::string& cigar);
public:
    static AlignmentPosition FindLocalAlignmentPositions(std::string_view read, std::string_view motif);
    //static std::vector<AlignmentPosition> FindAllBestLocalAlignmentPositions(std::string_view read, std::vector<std::string>& motifs, std::int64_t MAX_ED);
    static std::int32_t KeepReadAfter(std::string_view& read, std::vector<std::string>& motifs, std::int64_t& mtfIdx, std::int32_t MAX_ED);
    static std::int32_t KeepReadBefore(std::string_view& read, std::vector<std::string>& motifs, std::int64_t& mtfIdx, std::int32_t MAX_ED);

    static std::int32_t EditDistance(std::string_view read1, std::string_view read2, std::int32_t MAX_ED = -1);

    static std::int32_t CleanReadAfter(std::string& read, std::vector<std::string>& motifs, std::int64_t& mtfIdx, std::int32_t MAX_ED);
    static std::int32_t CleanReadBefore(std::string& read, std::vector<std::string>& motifs, std::int64_t& mtfIdx, std::int32_t MAX_ED);

    //static void ApplyCigar(std::string &read, std::string &cigar);
    //static std::int32_t GetCigarLocalAlignment(std::string_view oligo, std::string_view motif, std::string &cigar);


};


#endif //PRIMERSREMOVAL_ALIGN_H

