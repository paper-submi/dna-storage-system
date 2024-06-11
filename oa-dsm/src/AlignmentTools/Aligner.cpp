#include <algorithm>
#include "Aligner.h"
#include "edlib/include/edlib.h"

bool GetBestLocalAlignment(std::string_view read, std::vector<std::string>& motifs, AlignmentPosition& bestPos, std::int64_t& mtfIdx, std::int32_t& min_ed, std::int32_t MAX_ED){
    min_ed = MAX_ED;
    AlignmentPosition pos{-1, -1, -1};
    bool found = false;

    std::uint32_t idx = 0;
    for (auto &p: motifs) {
        pos = Aligner::FindLocalAlignmentPositions(read, p);
        std::int32_t ed = pos.editDistance;
        if (ed >= 0 && ed <= min_ed) {
            min_ed = ed;
            bestPos = pos;
            found = true;
            mtfIdx = idx;
        }
        idx++;
    }
    return found;
}

std::int32_t Aligner::EditDistance(std::string_view read1, std::string_view read2, std::int32_t MAX_ED){
    EdlibAlignResult result = edlibAlign(read1.data(), read1.size(), read2.data(), read2.size(), edlibNewAlignConfig(MAX_ED, EDLIB_MODE_NW, EDLIB_TASK_DISTANCE, NULL, 0));
    std::int32_t ED=-1;
    if (result.status == EDLIB_STATUS_OK) {
        ED=result.editDistance;
    }
    edlibFreeAlignResult(result);
    return ED;
}


AlignmentPosition Aligner::GetLocalAlignment(std::string_view oligo, std::string_view motif, std::string& cigar) {
    const char *oligoPtr = oligo.data();
    const char *primerPtr = motif.data();

    auto szOligo = static_cast<std::uint32_t>(oligo.size());
    size_t szMotif = static_cast<std::uint32_t>(motif.size());

    auto modeCode = EDLIB_MODE_HW;
    EdlibAlignResult result = edlibAlign(primerPtr, szMotif, oligoPtr, szOligo,
                                         edlibNewAlignConfig(-1, modeCode, EDLIB_TASK_PATH, NULL, 0));

    char *cigarPtr = edlibAlignmentToCigar(result.alignment, result.alignmentLength, EDLIB_CIGAR_STANDARD);

    std::int32_t ED = result.editDistance;

    std::int32_t endPos = result.endLocations[0];

    std::int32_t startPos = endPos;
    for (std::int32_t i = 0; i < result.alignmentLength; i++) {
        if (result.alignment[i] != EDLIB_EDOP_INSERT) {
            startPos--;
        }
    }

    cigar = cigarPtr;

    edlibFreeAlignResult(result);
    free(cigarPtr);
    return {startPos, endPos, ED};
}


AlignmentPosition Aligner::FindLocalAlignmentPositions(std::string_view read, std::string_view motif) {
    std::string cigar;
    auto alignment = GetLocalAlignment(read, motif, cigar);
    return {alignment.start + 1, alignment.end + 1, alignment.editDistance};
}

/*std::vector<AlignmentPosition> Aligner::FindAllBestLocalAlignmentPositions(std::string_view read, std::vector<std::string>& motifs, std::int64_t MAX_ED) {
    std::vector<AlignmentPosition> alnSet;
    std::int32_t MIN_ED = MAX_ED;
    for (auto& mtf : motifs) {
        auto aln = FindLocalAlignmentPositions(read, mtf);
        if (aln.editDistance < MIN_ED) {
            MIN_ED = aln.editDistance;
        }
        alnSet.emplace_back(aln);
    }
    auto endItr = std::remove_if(alnSet.begin(), alnSet.end(), [MIN_ED](auto& a){
        return a.editDistance != MIN_ED;
    });
    alnSet.erase(endItr, alnSet.end());
    return alnSet;
}*/

std::int32_t
Aligner::CleanReadBefore(std::string& read, std::vector<std::string>& motifs, std::int64_t& mtfIdx, std::int32_t MAX_ED) {
    AlignmentPosition bestPos{-1, -1, -1};
    std::int32_t min_ed;
    if (GetBestLocalAlignment(read, motifs, bestPos, mtfIdx, min_ed, MAX_ED)) {
        size_t start = bestPos.start;// < 0 ? 0 : bestPos.start;
        if ( start < read.size() ) {
            read = read.substr(start, std::string::npos);
            return min_ed;
        }
    }
    return -1;
}

std::int32_t
Aligner::CleanReadAfter(std::string& read, std::vector<std::string>& motifs, std::int64_t& mtfIdx, std::int32_t MAX_ED) {
    AlignmentPosition bestPos{-1, -1, -1};
    std::int32_t min_ed;
    if (GetBestLocalAlignment(read, motifs, bestPos, mtfIdx, min_ed, MAX_ED)) {
        if(bestPos.end>=0) {// if positive, staic cast for comparison with string::size
            size_t end = static_cast<size_t>(bestPos.end) >= read.size() ? std::string::npos : bestPos.end;
            read = read.substr(0, end);
            return min_ed;
        }
        // if negative, do nothing
    }
    return -1;
}

std::int32_t
Aligner::KeepReadBefore(std::string_view& read, std::vector<std::string>& motifs, std::int64_t& mtfIdx, std::int32_t MAX_ED) {
    AlignmentPosition bestPos{-1, -1, -1};
    std::int32_t min_ed;
    if (GetBestLocalAlignment(read, motifs, bestPos, mtfIdx, min_ed, MAX_ED)) {
        ssize_t end = bestPos.start;// < 0 ? 0 : bestPos.start;
        if ( end > 0 && end < static_cast<ssize_t>(read.size()) ) {
            read = read.substr(0, end);
            return min_ed;
        }
    }
    return -1;
}

std::int32_t
Aligner::KeepReadAfter(std::string_view& read, std::vector<std::string>& motifs, std::int64_t& mtfIdx, std::int32_t MAX_ED) {
    AlignmentPosition bestPos{-1, -1, -1};
    std::int32_t min_ed;
    if (GetBestLocalAlignment(read, motifs, bestPos, mtfIdx, min_ed, MAX_ED)) {
        ssize_t start = bestPos.end;// < 0 ? 0 : bestPos.start;
        if ( start > 0 && start < static_cast<ssize_t>(read.size()) ) {
            read = read.substr(start, std::string::npos);
            return min_ed;
        }
    }
    return -1;
}

/*
std::int32_t Aligner::GetCigarLocalAlignment(std::string_view oligo, std::string_view motif, std::string &cigar) {
    const char *oligoPtr = oligo.data();
    const char *motifPtr = motif.data();

    auto szOligo = static_cast<std::uint32_t>(oligo.size());
    size_t szMotif = static_cast<std::uint32_t>(motif.size());

    auto modeCode = EDLIB_MODE_HW;
    EdlibAlignResult result = edlibAlign(motifPtr, szMotif, oligoPtr, szOligo,
                                         edlibNewAlignConfig(-1, modeCode, EDLIB_TASK_PATH, NULL, 0));

    char *cigarPtr = edlibAlignmentToCigar(result.alignment, result.alignmentLength, EDLIB_CIGAR_STANDARD);

    std::int32_t ED = result.editDistance;
    int endPos = result.endLocations[0];
    int startPos = endPos;
    for (int i = 0; i < result.alignmentLength; i++) {
        if (result.alignment[i] != EDLIB_EDOP_INSERT) {
            startPos--;
        }
    }
    cigar = cigarPtr;

    edlibFreeAlignResult(result);
    free(cigarPtr);
    return ED;
}

*/
/*
void Aligner::ApplyCigar(std::string &read, std::string &cigar) {

    stringstream cigarStream(cigar);

    char op;
    std::uint32_t count;
    std::uint32_t editMismatches = 0;
    std::uint32_t refPos = 0, readPos = 0;
    size_t tot = 0;

    while (cigarStream >> count >> op) {
        switch (op) {
            case 'M':
                tot += count;
                break;
            case 'D':
                editMismatches += count;
                refPos += count;
                read.insert(tot, count, '-');
                tot += count;
                break;
            case 'I':
                editMismatches += count;
                readPos += count;
               // if ((tot + count) <
                 //   s.size()) {// We interpret only the last insertion as substitution, because 10x more likely
                read.erase(tot, count);
                //}
                break;
            default:
                assert(0);
        }
    }
}

 */

/*
bool
keep_read_before(string_view &read_to_process, std::vector<std::string> &motifs, std::size_t &best_motif_idx,
                 size_t MAX_ED) {

    int min_ed = MAX_ED;
    std::pair<int32_t, int32_t> min_pos(-1, -1);
    std::pair<int32_t, int32_t> pos;
    bool found = false;

    size_t i = 0;
    for (auto &p: motifs) {
        int ed = 0;

        pos = find_boundaries(read_to_process, p, ed);
        if (ed >= 0 && ed <= min_ed) {
            min_ed = ed;
            min_pos = pos;
            found = true;
            best_motif_idx = i;
        }
        i++;
    }
    if (found) {
        //size_t end = min_pos.second > read_to_process.size() ? read_to_process.size() : min_pos.second + 1;
        size_t end = min_pos.first < read_to_process.size() ? min_pos.first : read_to_process.size();
        //if ( (read_to_process.size() - end) > 0 ) {
        if (read_to_process.size() > end) {
            read_to_process = read_to_process.substr(0, end);
            return true;
        }
    }
    return false;
}


bool keep_read_after(string_view &read_to_process, std::vector<std::string> &motifs, std::size_t &best_motif_idx,
                     size_t MAX_ED) {

    int min_ed = MAX_ED;
    std::pair<int32_t, int32_t> min_pos(-1, -1);
    std::pair<int32_t, int32_t> pos;
    bool found = false;

    size_t i = 0;
    for (auto &p: motifs) {
        int ed = 0;

        pos = find_boundaries(read_to_process, p, ed);
        if (ed >= 0 && ed <= min_ed) {
            min_ed = ed;
            min_pos = pos;
            found = true;
            best_motif_idx = i;
        }
        i++;
    }
    if (found) {
        size_t start = min_pos.second > read_to_process.size() ? read_to_process.size() : min_pos.second;
        //size_t end = min_pos.second;
        if (read_to_process.size() > start) {
            read_to_process = read_to_process.substr(start, read_to_process.size());
            return true;
        }
    }
    return false;
}
 */