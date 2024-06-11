#ifndef REFACTORINGOLIGOARCHIVE_GENERIC_H
#define REFACTORINGOLIGOARCHIVE_GENERIC_H

#include <string>
#include "CodecConfiguration.h"
//CodecConfiguration CodecConfig;

void call_encode_ldpc(CodecConfiguration& config, const std::string &directory_path, std::string &input_filename, std::string &output_filename);

void call_decode_ldpc(CodecConfiguration& config, std::string &directory_path, std::string &llr_input_filename, std::string &ldpc_output_filename);



#endif //REFACTORINGOLIGOARCHIVE_GENERIC_H
