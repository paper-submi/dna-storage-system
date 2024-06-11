#include <iostream>
#include <fstream>
#include <tbb/parallel_for.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "Settings/CodecConfiguration.h"
#include "Settings/FileConfiguration.h"
#include "Settings/Constants.h"
#include "Settings/Generic.h"
#include "../../src/Common/Utils.h"
#include "../../src/Common/Logger.h"
#include "../../src/Codec/Encoder.h"
#include "../../src/DataStructure/Extent.h"
#include "../../src/DataStructure/File.h"


using namespace std;
namespace fs = boost::filesystem;

CodecConfiguration CodecConfig;
FileConfiguration FileConfig;

int main(int argc, char **argv) {

    namespace po = boost::program_options;
    bool help{};
    // bool is_binary = false;
    po::options_description description("encoder [options]");
    description.add_options()("help,h", po::bool_switch(&help), "Display help")(
            "filepath,f", po::value<string>(), "File to encode")(
            "params,p", po::value<string>(), "Read parameters from config file")(
            "config,g", po::value<string>(), "Path to configuration file"//)(
            //"binary,b", po::bool_switch(&binary)->default_value(false), "Print oligos as binary [default false]"
    );

    po::command_line_parser parser{argc, argv};
    parser.options(description);
    auto parsed_result = parser.run();
    po::variables_map vm;
    po::store(parsed_result, vm);
    po::notify(vm);

    InitLogger(debug);

    string param_filename;
    string config_filename;
    string input_file_path;

    if (vm.count("params") && vm.count("config") && vm.count("filepath")) {
        input_file_path = vm["filepath"].as<string>();
        param_filename = vm["params"].as<string>();
        config_filename = vm["config"].as<string>();
    } else {
        std::cerr << description << std::endl;
        return 1;
    }

    CodecConfig.init(config_filename);
    CodecConfig.print_configuration(cout);

    File fileToEncode(Settings::NUMBER_BLOCKS_PER_EXTENT, Settings::LDPC_DIM);

    fileToEncode.Open(input_file_path);

    size_t nItr = 0;

    while (fileToEncode.Read()) {
        /**
         * Randomize each block separately.
         * Write down on file the randomized blocks as a single binary string.
         */
        fileToEncode.Randomize();

        std::string input_filename = fs::path(input_file_path).filename().string();

        LOG(info) << "Start LDPC encoding" << std::endl;

        tbb::parallel_for(size_t(0), MAX_EXTENTS_IN_MEMORY, [&](size_t localExtIdx) {
            size_t extIdx = nItr * MAX_EXTENTS_IN_MEMORY + localExtIdx;
            if (extIdx < fileToEncode.GetNExtents()) {
                fs::path base_path(CodecConfig.TMP_ENCODER_DATA_PATH + "/ext_" + std::to_string(extIdx));

                fs::path boost_randomized_file_path = base_path / fs::path(input_filename + ".tmp").filename();
                fs::path boost_ldpc_encoded_file_path = base_path / fs::path(input_filename + ".ldpc").filename();

                std::string randomized_file_path = boost_randomized_file_path.string();
                std::string ldpc_encoded_file_path = boost_ldpc_encoded_file_path.string();

                fs::path tmp_data_dir_path(base_path);
                if (fs::exists(tmp_data_dir_path)) {//fs::is_directory( tmp_data_dir_path ) ){
                    if (!fs::remove_all(tmp_data_dir_path)) {
                        LOG(fatal) << "Cannot remove " << tmp_data_dir_path << ". Remove it manually and try again";
                        exit(-1);
                    }
                } else {
                    fs::create_directories(tmp_data_dir_path);
                }

                LOG(info) << "Encoding extent " << extIdx << std::endl;

                auto extent = fileToEncode.GetExtentsList()[localExtIdx];
                {
                    std::ofstream randFileStream(randomized_file_path);
                    for (const auto &block: extent.GetBlocks()) {
                        randFileStream << block.ToString();
                    }
                }
                /**
                 * Call the LDPC encoder on the randomized binary strings.
                 * This produces a set of 281600 bits long blocks.
                 **/
                std::string ldpc_encoder_input_filename = fs::path(input_file_path + ".tmp").filename().string();
                std::string ldpc_encoder_output_filename = fs::path(input_file_path + ".ldpc").filename().string();
                call_encode_ldpc(CodecConfig, CodecConfig.TMP_ENCODER_DATA_PATH + "/ext_" + std::to_string(extIdx),
                                 ldpc_encoder_input_filename,
                                 ldpc_encoder_output_filename);
            }

        });

        LOG(info) << "End LDPC encoding" << std::endl;

        LOG(info) << "Reading LDPC blocks" << std::endl;
        /**
         * Load the LDPC encoded blocks from file.
         * Each block is extended to be a multiple of 15 bits.
         * In this case, a block of 281600 (10% redundancy) it will become 281610
         */
        File encodedFile(Settings::NUMBER_BLOCKS_PER_EXTENT, Settings::FINAL_SIZE_LDPC_BLOCK);
        /**
        * Import metadata: it's important so that the same seed can be used to radnomize the blocks
        * */
        encodedFile.ImportMetadata(fileToEncode);

        fileToEncode.GetExtentsList().clear();

        for (size_t extIdx = nItr * MAX_EXTENTS_IN_MEMORY; extIdx < nItr * MAX_EXTENTS_IN_MEMORY + MAX_EXTENTS_IN_MEMORY; extIdx++) {
            if (extIdx < fileToEncode.GetNExtents()) {
                fs::path base_path(CodecConfig.TMP_ENCODER_DATA_PATH + "/ext_" + std::to_string(extIdx));
                fs::path boost_ldpc_encoded_file_path = base_path / fs::path(input_filename + ".ldpc").filename();
                std::string ldpc_encoded_file_path = boost_ldpc_encoded_file_path.string();
                std::string line;
                std::ifstream blockStream(ldpc_encoded_file_path);
                while (std::getline(blockStream, line)) {
                    encodedFile.AppendBlock(line);
                }
                fs::remove_all(base_path); // Clean the directory after reading it
            }
        }

        string left_primer_filename = CodecConfig.LEFT_PRIMERS_PATH;
        string right_primer_filename = CodecConfig.RIGHT_PRIMERS_PATH;
        vector<string> left_primers;
        vector<string> right_primers;

        LOG(info) << "Initializing encoder";
        /**
         * Inizialize the oligo encoder: load table and set all parameters.
         */
        Codec codec(Settings::NTS, Settings::PREFIX_SIZE, CodecConfig.TABLE_PATH);

        LOG(info) << "Tables loaded correctly";

        LOG(debug) << "Generating encoding motifSet" << std::endl;

        size_t primers_idx = 0;

        ColumnarSerializer serializer(Settings::BITVALUE_SIZE, Settings::INDEX_LEN);

        tbb::parallel_for(size_t(0), MAX_EXTENTS_IN_MEMORY, [&](size_t localExtIdx) {
            Encoder encoder(codec);
            size_t extIdx = nItr * MAX_EXTENTS_IN_MEMORY + localExtIdx;
            if (extIdx < encodedFile.GetNExtents()) {
                Extent &extent = encodedFile.GetExtentsList()[localExtIdx];
                std::vector<Motif> motifSet;
                std::vector<Oligo> oligos;

                fs::path output_oligos_files_path(
                        CodecConfig.DATA_DIR_PATH + "/" + input_filename + ".EXTENT_" + to_string(extIdx) +
                        ".FASTA");
                output_oligos_files_path.normalize();

                std::vector<std::vector<int64_t>> data;
                data = extent.SerializeData(serializer);
                oligos.resize(data[0].size());

                for (size_t mtfIdx = 0; mtfIdx < Settings::NUMBER_OF_MOTIFS; mtfIdx++) {
                    encoder.Fill(data[mtfIdx], Settings::PREFIX_SIZE, Settings::NTS);
                    encoder.EncodeValuesGroup();
                    encoder.GetEncodingMotifs(motifSet);

                    for (size_t oIdx = 0; oIdx < motifSet.size(); oIdx++) {
                        oligos[oIdx].AppendMotif(motifSet[oIdx]);
                    }
                    motifSet.clear();
                }


                ofstream oligos_file(output_oligos_files_path.c_str());
                size_t refIdx = extIdx * oligos.size();
                for (auto &oligo: oligos) {
                    oligos_file << ">ref" << refIdx << std::endl;
                    oligos_file << oligo.toString() << std::endl;
                    refIdx++;
                }


                LOG(info) << "Encoding oligos written in "
                          << CodecConfig.DATA_DIR_PATH + "/" + input_filename + ".EXTENT_" + to_string(extIdx) +
                             //"_" + to_string(primers_idx) +
                             ".FASTA" << std::endl;
                primers_idx++;
                encoder.Clear();
            }
        });
        nItr++;
    }

    /**
         * Save the parameters for the encoded file
         **/
    LOG(info) << "Saving parameters for encoded file" << std::endl;
    {
        FileConfig.ORIGINAL_FILE_SIZE = fileToEncode.GetSzFile();
        FileConfig.RAND_SEED = fileToEncode.GetRandSeed();
        FileConfig.MD5 = fileToEncode.GetMd5();
        FileConfig.INPUT_FILENAME = fileToEncode.GetInputFilename();
        FileConfig.EXT = fileToEncode.GetExt();
        FileConfig.N_EXTENTS = fileToEncode.GetNExtents();
    }

    { /** Save parameters for encoded file */
        ofstream out_conf(param_filename);
        FileConfig.print_configuration(out_conf);
        LOG(info) << "Configuration file for encoded data written in " << param_filename;
    }
    return 0;
}