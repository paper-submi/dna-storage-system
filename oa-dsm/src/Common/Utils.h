#ifndef CODEC_UTILS_H
#define CODEC_UTILS_H

#include <string>
#include <vector>

void import_primers(std::vector<std::string> &primers, std::string &filename_primer);

void exec_with_output( std::vector<char *> &args, std::string &output_buffer );

void print_statistics(std::vector<std::vector<size_t>>& motifs, std::ostream &out);

template <typename T, class Comp>
size_t count_differences( std::vector<T> &v1, std::vector<T> &v2, Comp eq )
{
    assert(v1.size()==v2.size());
    size_t diff=0;
    for(size_t i=0; i<v1.size(); i++){
        diff += ( eq(v1[i], v2[i])?0:1 );
    }
    return diff;
}
#endif //CODEC_UTILS_H
