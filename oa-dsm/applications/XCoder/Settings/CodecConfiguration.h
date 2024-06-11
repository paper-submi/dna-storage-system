#ifndef CONSENSUS_CODECCONFIGURATION_H
#define CONSENSUS_CODECCONFIGURATION_H

#include <cstddef>
#include <string>
#include <iostream>

class CodecConfiguration {
    void inizialize_data_directories();

public:

    std::string LDPC_ENCODE_PATH;// = "../LDPC-codes/encode";
    std::string LDPC_DECODE_PATH;// = "../LDPC-codes/decode";
    std::string LDPC_EXTRACT_PATH;// = "../LDPC-codes/extract";
    std::string LDPC_PREFIX;
    std::string TABLE_PATH;
    std::string LEFT_PRIMERS_PATH;
    std::string RIGHT_PRIMERS_PATH;
    std::string DATA_DIR_PATH;
    std::string EXTENTS_DATA_DIR_PATH;
    std::vector<std::string> TMP_DATA_DIR_PATH;
    std::string TMP_ENCODER_DATA_PATH;
    std::string DECODER_INPUT_PATH; // Clusters path

    size_t LDPC_MAX_ITER;

    void init(const std::string &config_filename);

    void print_configuration(std::ostream &out) const;
};

#endif //CONSENSUS_CODECCONFIGURATION_H
