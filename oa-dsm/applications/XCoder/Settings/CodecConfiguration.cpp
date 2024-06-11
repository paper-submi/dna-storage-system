#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>
#include "CodecConfiguration.h"
#include "../../../src/Common/Logger.h"

using namespace std;
namespace fs = boost::filesystem;

void clean_path(std::string &final_path) {
    fs::path generic_path(final_path);
    generic_path = generic_path.remove_trailing_separator().normalize();
    final_path = generic_path.string();
}

void CodecConfiguration::init(const std::string &config_filename) {

    string key, skip, value;

    ifstream config(config_filename);

    if (!config.is_open()) {
        cout << "ERROR: No config file available." << std::endl;
        exit(-1);
    }
    string line;
    while (getline(config, line)) {
        if(line[0]=='#' || line.empty()) {
            continue;
        }
        stringstream sline(line);
        sline >> key >> skip >> value;

        if (key == "LDPC_PREFIX") {
            this->LDPC_PREFIX = value;
        } else if (key == "LDPC_MAX_ITER") {
            this->LDPC_MAX_ITER = stoul(value);
        } else if (key == "TABLE_PATH") {
            this->TABLE_PATH = value;
        } else if (key == "DECODER_INPUT_PATH") {
            this->DECODER_INPUT_PATH = value;
        } else if (key == "LEFT_PRIMERS_PATH") {
            this->LEFT_PRIMERS_PATH = value;
        } else if (key == "RIGHT_PRIMERS_PATH") {
            this->RIGHT_PRIMERS_PATH = value;
        } else if (key == "DATA_DIR_PATH") {
            this->DATA_DIR_PATH = value;
        } else if (key == "EXTENTS_DATA_DIR_PATH") {
            this->EXTENTS_DATA_DIR_PATH = value;
        } else if (key == "TMP_ENCODER_DATA_PATH") {
            this->TMP_ENCODER_DATA_PATH = value;
        } else if (key == "LDPC_ENCODE_PATH") {
            this->LDPC_ENCODE_PATH = value;
        } else if (key == "LDPC_DECODE_PATH") {
            this->LDPC_DECODE_PATH = value;
        } else if (key == "LDPC_EXTRACT_PATH") {
            this->LDPC_EXTRACT_PATH = value;
        }
    }

    clean_path(this->EXTENTS_DATA_DIR_PATH);
    clean_path(this->DATA_DIR_PATH);
    clean_path(this->LEFT_PRIMERS_PATH);
    clean_path(this->RIGHT_PRIMERS_PATH);
    clean_path(this->DECODER_INPUT_PATH);
    clean_path(this->TABLE_PATH);
    clean_path(this->LDPC_PREFIX);

    inizialize_data_directories();
}

void CodecConfiguration::print_configuration(ostream &out) const {
    out << "LDPC_PREFIX = " << this->LDPC_PREFIX << std::endl;
    out << "LDPC_MAX_ITER = " << this->LDPC_MAX_ITER << std::endl;
    out << "TABLE_PATH = " << this->TABLE_PATH << std::endl;
    out << "DECODER_INPUT_PATH = " << this->DECODER_INPUT_PATH << std::endl;
    out << "LEFT_PRIMERS_PATH = " << this->LEFT_PRIMERS_PATH << std::endl;
    out << "RIGHT_PRIMERS_PATH = " << this->RIGHT_PRIMERS_PATH << std::endl;
    out << "DATA_DIR_PATH = " << this->DATA_DIR_PATH << std::endl;
    out << "EXTENT_DATA_DIR_PATH = " << this->EXTENTS_DATA_DIR_PATH << std::endl;
    out << "TMP_ENCODER_DATA_PATH = " << this->TMP_ENCODER_DATA_PATH << std::endl;
    out << "LDPC_ENCODE_PATH = " << this->LDPC_ENCODE_PATH << std::endl;
    out << "LDPC_DECODE_PATH = " << this->LDPC_DECODE_PATH << std::endl;
    out << "LDPC_EXTRACT_PATH = " << this->LDPC_EXTRACT_PATH << std::endl;
}

void CodecConfiguration::inizialize_data_directories() {

    fs::path data_dir_path(this->DATA_DIR_PATH);
    if (!fs::exists(data_dir_path) || !fs::is_directory(data_dir_path)) {
        LOG(fatal) << "Cannot remove " << data_dir_path << ". Directory doesn't exist.";
        //fs::create_directories(data_dir_path);
    }
    fs::path tmp_data_dir_path(this->TMP_ENCODER_DATA_PATH);
    if (fs::exists(tmp_data_dir_path)) {//fs::is_directory( tmp_data_dir_path ) ){
        if (!fs::remove_all(tmp_data_dir_path)) {
            LOG(fatal) << "Cannot remove " << tmp_data_dir_path << ". Remove it manually and try again";
            exit(-1);
        }
    }
    fs::create_directories(tmp_data_dir_path);
}
