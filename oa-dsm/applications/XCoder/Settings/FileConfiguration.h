#ifndef CONSENSUS_FILECONFIGURATION_H
#define CONSENSUS_FILECONFIGURATION_H

#include <string>

class FileConfiguration {
public:
    std::string INPUT_FILENAME;
    //std::string OUTPUT_FILENAME;
    std::string EXT;
    std::string MD5;
    size_t RAND_SEED;
    //size_t INDEX_LEN;
    size_t ORIGINAL_FILE_SIZE;
    size_t ENCODED_FILE_SIZE;
    size_t N_EXTENTS;

    void init(std::string param_filename);

    void print_configuration(std::ostream &out);

};

#endif //CONSENSUS_FILECONFIGURATION_H
