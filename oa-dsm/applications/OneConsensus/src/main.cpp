#include "onecluster.hpp"
#include "utils.hpp"
#include "constants.hpp"
#include "onejoin.hpp"
#include "Time.hpp"
#include "../../../src/Common/Logger.h"

using namespace std;

int main(int argc, char **argv) {

    bool help{};
    namespace po = boost::program_options;

    std::string description_string;

#ifdef SHORT_READ
    description_string="oneconsensus_shortreads [options]";
#endif

#ifdef LONG_READ
    description_string="oneconsensus_longreads [options]";
#endif

    po::options_description description(description_string);

    init_options(description, help);

    po::command_line_parser parser{argc, argv};

    parser.options(description);

    auto parsed_result = parser.run();

    po::variables_map vm;

    po::store(parsed_result, vm);

    po::notify(vm);

    string filename = "";

    uint32_t samplingrange = 0; // The maximum digit to embed, the range to sample

    bool debug = false;
    size_t rigth_length = 0;
    size_t offset = 0;

    if (help) {
        std::cerr << description << std::endl;
        return 0;
    }

    if ( vm.count("read") && /*vm.count("device") &&*/
        vm.count("length") ) {
        filename = vm["read"].as<string>();
        rigth_length = vm["length"].as<size_t>();
        debug = vm["verbose"].as<bool>();
    } else {
        std::cerr << description << std::endl;
        return 1;
    }

    if( vm.count("offset") ){
        offset = vm["offset"].as<uint32_t>();
    }

    InitLogger(debug?LoggerLevel::debug:LoggerLevel::info);

    samplingrange = (rigth_length-1)<=0?1:(rigth_length*3);

    string dataset_name="";
    if (vm.count("dataset_name")) {
        dataset_name=vm["dataset_name"].as<string>();
    }

    if(constants::NUM_HASH != 1 || constants::NUM_STR!=1){
        LOG(error)<<"The current implementation does not allow to use more than 1 random strings and/or 1 hash functions.";
        exit(-1);
    }

    InputData input_data;
    read_dataset(input_data, filename);

    OutputValues output_val;
    Time timer((alg::cluster));

    one_consensus(input_data, rigth_length, offset, samplingrange, timer, dataset_name);

    save_parameters("experiment-parameters", argc, argv, filename, rigth_length, offset, input_data);
    save_report( dataset_name, output_val, timer );

   return 0;
}
