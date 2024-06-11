#include <boost/filesystem.hpp>
#include "Generic.h"
#include "Constants.h"
#include "../../../src/Common/Utils.h"
#include "../../../src/Common/Logger.h"

namespace fs = boost::filesystem;

void call_encode_ldpc(CodecConfiguration& config, const std::string &directory_path, std::string &input_filename, std::string &output_filename) {
    LOG(info) << "Calling LDPC";
    LOG(info) << "\tEncoding";

    fs::path input_path = fs::path(directory_path) / fs::path(input_filename).filename();
    fs::path output_path = fs::path(directory_path) / fs::path(output_filename).filename();

    std::string output_cmd;

    std::vector<char *> args;
    std::string arg0 = config.LDPC_ENCODE_PATH;
    std::string arg1 = config.LDPC_PREFIX + ".pchk";
    std::string arg2 = config.LDPC_PREFIX + ".gen";
    std::string arg3 = input_path.string();
    std::string arg4 = output_path.string();

    args.emplace_back((char *) arg0.c_str());
    args.emplace_back((char *) arg1.c_str());
    args.emplace_back((char *) arg2.c_str());
    args.emplace_back((char *) arg3.c_str());
    args.emplace_back((char *) arg4.c_str());
    args.push_back(nullptr);

    exec_with_output(args, output_cmd);

    LOG(info) << "\t" << output_cmd;
}

void call_decode_ldpc(CodecConfiguration& config, std::string &directory_path, std::string &llr_input_filename, std::string &ldpc_output_filename) {

    std::string input_path = fs::path(fs::path(directory_path) / fs::path(llr_input_filename).filename()).string();
    std::string ldpc_intermediate_output_path = (fs::path(directory_path) / "data.dec.tmp").string();
    std::string output_path = fs::path(fs::path(directory_path) / fs::path(ldpc_output_filename).filename()).string();

    LOG(info) << "Calling LDPC";
    LOG(info) << "\tDecoding";
    std::string info_string =
            "\t" + config.LDPC_DECODE_PATH + " " + config.LDPC_PREFIX + ".pchk " + input_path + " " +
            ldpc_intermediate_output_path;
    LOG(debug) << info_string;

    std::string output_cmd;
    {
        std::vector<char *> args;

        std::string ldpc_decoder_path = config.LDPC_DECODE_PATH;
        std::string ldpc_parity_check_path = config.LDPC_PREFIX + ".pchk";
        std::string ldpc_decoded_output_path = ldpc_intermediate_output_path;
        std::string ldpc_max_iteration = std::to_string(config.LDPC_MAX_ITER);

        args.push_back((char *) ldpc_decoder_path.c_str());
        args.push_back((char *) ldpc_parity_check_path.c_str());
        args.push_back((char *) input_path.c_str());
        args.push_back((char *) ldpc_decoded_output_path.c_str());
        args.push_back((char *) "misc");
        args.push_back((char *) "0.0");
        args.push_back((char *) "prprp");
        args.push_back((char *) ldpc_max_iteration.c_str());
        args.push_back(nullptr);

        exec_with_output(args, output_cmd);
    }

    LOG(info) << "\t" << output_cmd;

    LOG(info) << "\tExtracting";
    info_string = "\t" + config.LDPC_EXTRACT_PATH + " " + config.LDPC_PREFIX + ".gen " +
                  ldpc_intermediate_output_path + " " + output_path;
    LOG(debug) << info_string;

    output_cmd="";

    {
        std::vector<char *> args;

        std::string ldpc_gen_matrix_path = config.LDPC_PREFIX + ".gen";
        std::string ldpc_extract_path = config.LDPC_EXTRACT_PATH;
        std::string ldpc_input_path = ldpc_intermediate_output_path;

        args.push_back((char *) ldpc_extract_path.c_str());
        args.push_back((char *) ldpc_gen_matrix_path.c_str());
        args.push_back((char *) ldpc_input_path.c_str());
        args.push_back((char *) output_path.c_str());
        args.push_back(nullptr);


        exec_with_output(args, output_cmd);
    }
    LOG(info) << "\t" << output_cmd;

    fs::remove_all(ldpc_intermediate_output_path);
    fs::remove_all(input_path);
}