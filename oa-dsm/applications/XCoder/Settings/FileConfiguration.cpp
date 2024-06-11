#include <fstream>
#include <iostream>
#include "FileConfiguration.h"

using namespace std;

void FileConfiguration::init(std::string param_filename) {

    string key, skip, value;

    ifstream config(param_filename);

    if (!config.is_open()) {
        cout << "ERROR: No config file available." << std::endl;
        exit(-1);
    }

    while (!config.eof()) {

        config >> key >> skip >> value;

        if (key == "INPUT_FILENAME") {
            this->INPUT_FILENAME = value;
        } /*else if (key == "OUTPUT_FILENAME") {
            this->OUTPUT_FILENAME = value;
        }*/ else if (key == "MD5") {
            this->MD5 = value;
        } else if (key == "RAND_SEED") {
            this->RAND_SEED = stoull(value);
        } else if (key == "EXT") {
            this->EXT = value;
        } else if (key == "ORIGINAL_FILE_SIZE") {
            this->ORIGINAL_FILE_SIZE = stoull(value);
        } else if (key == "ENCODED_FILE_SIZE") {
            this->ENCODED_FILE_SIZE = stoull(value);
        } else if (key == "N_EXTENTS") {
            //this->INDEX_LEN = stoull(value);
            this->N_EXTENTS = stoull(value);
        }
    }
}


void FileConfiguration::print_configuration(std::ostream &out) {
    out << "INPUT_FILENAME = " << this->INPUT_FILENAME << std::endl;
    //out << "OUTPUT_FILENAME = " << this->OUTPUT_FILENAME << std::endl;
    out << "MD5 = " << this->MD5 << std::endl;
    out << "RAND_SEED = " << this->RAND_SEED << std::endl;
    out << "EXT = " << this->EXT << std::endl;
    out << "ORIGINAL_FILE_SIZE = " << this->ORIGINAL_FILE_SIZE << std::endl;
    out << "ENCODED_FILE_SIZE = " << this->ENCODED_FILE_SIZE << std::endl;
    out << "N_EXTENTS = " << this->N_EXTENTS << std::endl;
}

