#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "Settings/Constants.h"
#include "Settings/FileConfiguration.h"
#include "Settings/Generic.h"
#include "../../src/Codec/Decoder.h"
#include "../../src/Codec/Encoder.h"
#include "../../src/Common/Utils.h"
#include "../../src/Common/Logger.h"
#include "../../src/DataStructure/Extent.h"
#include "../../src/AlignmentTools/Aligner.h"
#include "../../src/DataStructure/Cluster.h"
#include "../../src/DataStructure/File.h"

//#define DEBUG
//#define FOR_TESTING_ONLY
#define PARALLEL

using namespace std;
namespace fs = boost::filesystem;

CodecConfiguration CodecConfig;
FileConfiguration FileConfig;

std::vector<std::vector<size_t>> corrected_motifs;


struct Column{
    std::uint64_t N{};
    std::vector<Motif> motif;
    std::vector<std::int64_t> value;
    std::vector<std::int64_t> indexes;
    std::vector<float> llr;
    std::vector<bool> isValid;
    void Resize(std::uint64_t nElements){
        motif.resize(nElements, Motif{0, 0});
        value.resize(nElements, -1);
        llr.resize(nElements, 0);
        isValid.resize(nElements, false);
        indexes.resize(nElements, -1);
        N = nElements;
    }
    void GetIndexes( std::uint32_t szIndex ){
        for (std::uint32_t idx = 0; idx < value.size(); idx++) {
            indexes[idx] = 0;
            std::int64_t tmpIndex = value[idx];
            for (size_t b = 0; b < szIndex; b++) {
                indexes[idx] <<= 1;
                indexes[idx] += tmpIndex & 0x1;
                tmpIndex >>= 1;
            }
        }
    }
    std::uint64_t CountNotValidMotifs() {
        std::uint32_t count = 0;
        for (auto valid : isValid) {
            count += valid ? 0 : 1;
        }
        return count;
    }

    [[nodiscard]] std::uint64_t Size() const{
        return N;
    }

    void Clear(){
        motif.clear();
        value.clear();
        llr.clear();
        isValid.clear();
        indexes.clear();
        N = 0;
    }
};
void WriteLLR(std::ofstream& fileStreamLLR, BitVector& blockLDPC, std::vector<float>& coefficient, std::uint32_t startIdx, std::uint32_t szData, std::uint32_t BLOCK_SIZE);
void DecodeExtent(Decoder decoder, Encoder encoder, size_t extent_idx);
void AdjustOligos(Column& previousColumn, Column& correctedColumn, std::vector<ClusterView>& clusters, size_t mtfIdx);

int main(int argc, char **argv) {

    namespace po = boost::program_options;
    bool help{};
    po::options_description description("decoder [options]");
    description.add_options()("help,h", po::bool_switch(&help), "Display help")(
            "params,p", po::value<std::string>(), "Read parameters from config file")(
            "config,g", po::value<std::string>(), "Path to configuration file")(
            "extent,e", po::value<int64_t>(), "Limit decoding to one extent [optional]")(
            "verbose,v", po::bool_switch()->default_value(false), "[optional] Print all debug information"
    );

    po::command_line_parser parser{argc, argv};
    parser.options(description);
    auto parsed_result = parser.run();
    po::variables_map vm;
    po::store(parsed_result, vm);
    po::notify(vm);

    std::string param_filename;
    std::string config_filename;

    if (vm.count("params") && vm.count("config")) {
        param_filename = vm["params"].as<std::string>();
        config_filename = vm["config"].as<std::string>();
    } else {
        std::stringstream ssrt;
        ssrt << description;
        LOG(info) << ssrt.str();
        return 1;
    }

    int64_t selectedExtent = -1;

    if (vm.count("extent")) {
        selectedExtent = vm["extent"].as<int64_t>();

    }
    FileConfig.init(param_filename);
    CodecConfig.init(config_filename);
    FileConfig.print_configuration(std::cout);
    CodecConfig.print_configuration(std::cout);

    Codec codec(Settings::NTS, Settings::PREFIX_SIZE, CodecConfig.TABLE_PATH);
    Decoder decoder(codec);
    Encoder encoder(codec);

    LOG(info) << "Tables loaded correctly";

    LOG(info) << "Initializing tmp directories";

    for (size_t extent_idx = 0; extent_idx < FileConfig.N_EXTENTS; extent_idx++) {
        CodecConfig.TMP_DATA_DIR_PATH.emplace_back(
                CodecConfig.EXTENTS_DATA_DIR_PATH + "/ext_" + std::to_string(extent_idx) + "/tmp_data");
        fs::path tmp_data_dir_path(CodecConfig.TMP_DATA_DIR_PATH.back());
        if (fs::exists(tmp_data_dir_path)) {
            if (!fs::remove_all(tmp_data_dir_path)) {
                LOG(fatal) << "Cannot remove " << tmp_data_dir_path
                                         << ". Remove it manually and try again";
                exit(-1);
            }
        }
        fs::create_directories(tmp_data_dir_path);
    }

    corrected_motifs.resize(FileConfig.N_EXTENTS, std::vector<size_t>(Settings::NUMBER_OF_MOTIFS, -1) );

    LOG(info) << "Start decoding";

    size_t MAX_EXTENTS_IN_MEMORY = std::thread::hardware_concurrency();
    size_t MAX_ITR = (FileConfig.N_EXTENTS + MAX_EXTENTS_IN_MEMORY - 1 ) / MAX_EXTENTS_IN_MEMORY;

    for (size_t nItr = 0; nItr < MAX_ITR; nItr++) {
        try {
#ifdef PARALLEL
            tbb::parallel_for (static_cast<size_t>(0), MAX_EXTENTS_IN_MEMORY, [&](size_t localExtIdx){
#else
            for (size_t localExtIdx = 0; localExtIdx < MAX_EXTENTS_IN_MEMORY; localExtIdx++) {
#endif
                size_t extentIdx = nItr * MAX_EXTENTS_IN_MEMORY + localExtIdx;
                if (extentIdx < FileConfig.N_EXTENTS) {
                    if (selectedExtent != -1) {
                        if( extentIdx == selectedExtent) {
                            DecodeExtent(decoder, encoder, extentIdx);
                        }
                    } else {
                        DecodeExtent(decoder, encoder, extentIdx);
                    }
                }
            }
#ifdef PARALLEL
            );
#endif
        } catch (std::exception& e) {
            LOG(fatal) << e.what();
        }
    }

    LOG(debug) << "Reading decoded blocks";

    File outFile(Settings::NUMBER_BLOCKS_PER_EXTENT, Settings::LDPC_DIM);
    outFile.SetInputFilename(FileConfig.INPUT_FILENAME);
    outFile.SetSzFile(FileConfig.ORIGINAL_FILE_SIZE);
    outFile.SetExt(FileConfig.EXT);
    outFile.SetMd5(FileConfig.MD5);
    outFile.SetRandSeed(outFile.GetMd5());
    outFile.SetNExtents(FileConfig.N_EXTENTS);

    for (size_t extent_idx = 0; extent_idx < FileConfig.N_EXTENTS; extent_idx++) {
        size_t blockIdx = 0;
        for (size_t column_idx = 0; column_idx < Settings::NUMBER_OF_MOTIFS; column_idx++) {
            std::string line;
            std::ifstream decodedBlockStream(CodecConfig.TMP_DATA_DIR_PATH[extent_idx] + "/data.bin.tmp.col" + to_string(column_idx));
            while ( decodedBlockStream >> line ) {
                outFile.AppendBlock(line);
                blockIdx++;
            }
        }
        //assert(blockIdx == Settings::NUMBER_BLOCKS_PER_EXTENT);
    }

    print_statistics(corrected_motifs, std::cout);

    LOG(info) << "Derandomize data";
    outFile.Randomize();

    LOG(info) << "Writing out data in " << CodecConfig.DATA_DIR_PATH << "/"
                            << fs::path(FileConfig.INPUT_FILENAME).filename().string() << ".decoded";

    outFile.Write(CodecConfig.DATA_DIR_PATH + "/" + fs::path(FileConfig.INPUT_FILENAME).filename().string() + ".decoded");
    return 0;
}

void DecodeExtent(Decoder decoder, Encoder encoder, size_t extent_idx) {

    std::string LOCAL_EXTENT_DIR_PATH = "ext_" + std::to_string(extent_idx);

    ClustersDataset clustersDataset;

    std::vector<ClusterView> allClusters;
    Column fullColumn;

    std::vector<ClusterView> selectedClusters;
    Column selectedColumn;

    std::string cluster_file_path =
            CodecConfig.EXTENTS_DATA_DIR_PATH + "/" + LOCAL_EXTENT_DIR_PATH + "/" + CodecConfig.DECODER_INPUT_PATH;
    LOG(info) << "Reading oligos from " << cluster_file_path << std::endl;

    try {
        ClustersDataset::ImportClusters(cluster_file_path, clustersDataset, Settings::NTS);
    } catch (std::exception& e) {
        LOG(fatal) << e.what();
        exit(-1);
    }


    for (size_t idx = 0; idx < clustersDataset.Size(); idx++) {
        allClusters.emplace_back(clustersDataset.GetView(idx));
    }

    fullColumn.Resize(allClusters.size());

    size_t max_idx = Settings::FINAL_SIZE_LDPC_BLOCK / (Settings::BITVALUE_SIZE -
                                                        Settings::INDEX_LEN);
    selectedClusters.resize(max_idx);
    selectedColumn.Resize(max_idx);

    for (auto & allCluster : allClusters) {
        if (!allCluster.CheckPointersValidity()) {
            throw std::exception();
        }
    }

    for( std::uint64_t clusterIdx = 0; clusterIdx < allClusters.size(); clusterIdx++ ){
        ConsensusResult result = allClusters[clusterIdx].GetConsensus(Settings::NTS);
        if( !result.GetMotif().empty()/* && result.GetCoefficientLLR() != 0*/ ) {
            fullColumn.motif[clusterIdx] = Motif{result.GetMotif()};
            fullColumn.llr[clusterIdx] = result.GetCoefficientLLR();
            fullColumn.isValid[clusterIdx] = true;
        }else{
            LOG(warning) << "Cluster "<< clusterIdx << " gave invalid consensus at first iteration";
            // LOG(warning) << "Centroid: " << result.GetMotif();
            // LOG(warning) << "LLR coefficient: " << result.GetCoefficientLLR();
            // LOG(warning) << "Cluster size: " << allClusters[clusterIdx].Size();
        }
    }

    LOG(debug) << "\tRead " << allClusters.size() << " clusters" << std::endl;

    /**
     * At the first round of encoding, when it is not known the missing oligos, we assume that all encoding oligos are
     * valid. During the following decoding round ( next columns ) the missing oligos and corresponding decoding values
     * will be set as NOT valid.
     */
    for(size_t i = 0; i < fullColumn.Size(); i++) {
        fullColumn.isValid[i] = true;
    }

    bool is_index_column;

    std::string llr_filename = fs::path(fs::path(CodecConfig.TMP_DATA_DIR_PATH[extent_idx]) / "data.llr.tmp").string();

    /**
     * inferred_data and inferred_oligos will be used from the second decoding round on.
     * They assume that the oligo/value in a certain position is NOT valid, while it became valid if an index is found
     * for a certain position during the first round of decoding.
     */
    for (size_t columnIdx = 0; columnIdx < Settings::NUMBER_OF_MOTIFS; columnIdx++) {

        BitVector columnData(Settings::FINAL_SIZE_LDPC_BLOCK*2);

        std::ofstream llr_filestream(llr_filename);

        is_index_column = (columnIdx == Settings::IDX_OFFSET);

        if (is_index_column) {
            decoder.Fill(fullColumn.motif, Settings::PREFIX_SIZE);
            decoder.DecodeMotifsGroup();
            decoder.GetDecodedValues(fullColumn.value);

            fullColumn.GetIndexes(Settings::BITVALUE_SIZE - Settings::INDEX_LEN);

            size_t oligo_num = 0;
            size_t idx = 0;
            for (const auto& entryIdx : fullColumn.indexes) {
                idx = entryIdx;
                if (idx < max_idx) {
                    /** Directly get the inferred bit with the highest number of points */
                    if (allClusters[oligo_num].Size() > selectedClusters[idx].Size()) {
                        selectedClusters[idx] = allClusters[oligo_num];
                        selectedColumn.motif[idx] = fullColumn.motif[oligo_num];
                        selectedColumn.value[idx] = fullColumn.value[oligo_num];
                        selectedColumn.llr[idx] = fullColumn.llr[oligo_num];
                        selectedColumn.isValid[idx] = true;
                    }
                }
                oligo_num++;
            }

            allClusters.clear();
            fullColumn.Clear();

            LOG(info) << "Empty oligos: " << selectedColumn.CountNotValidMotifs();
        } else {

            //std::ofstream debug_column_consensus("col_consensus" + std::to_string(columnIdx));

            std::uint64_t clusterIdx=0;
            for (auto& cluster : selectedClusters) {
                auto result = cluster.GetConsensus(Settings::NTS);
                selectedColumn.motif[clusterIdx] = Motif{result.GetMotif(), Settings::NTS};
                selectedColumn.llr[clusterIdx] = result.GetCoefficientLLR();
                clusterIdx++;
                //debug_column_consensus << result.GetMotif() << " " << result.GetCoefficientLLR() << std::endl;
            }
            decoder.Fill(selectedColumn.motif, Settings::PREFIX_SIZE);
            decoder.DecodeMotifsGroup();
            decoder.GetDecodedValues(selectedColumn.value);
        }

        std::vector<BitVector> blocksLDPC;
        if (is_index_column) {
            /* Serialize the data not counting the index bits */
            blocksLDPC.emplace_back(Serializer::DeserializeVector(selectedColumn.value, Settings::BITVALUE_SIZE - Settings::INDEX_LEN, Settings::INDEX_LEN));
        }else{
            auto tmpBlocks = Serializer::DeserializeVector(selectedColumn.value, Settings::BITVALUE_SIZE, 0);
            blocksLDPC.emplace_back(tmpBlocks.ExtractBits(0, Settings::FINAL_SIZE_LDPC_BLOCK));
            blocksLDPC.emplace_back(tmpBlocks.ExtractBits(Settings::FINAL_SIZE_LDPC_BLOCK, Settings::FINAL_SIZE_LDPC_BLOCK));
        }
        std::uint32_t szData = (is_index_column ? Settings::BITVALUE_SIZE - Settings::INDEX_LEN : Settings::BITVALUE_SIZE);

        std::uint32_t startIdx = 0;
        for( auto& block : blocksLDPC ) {
            WriteLLR(llr_filestream, block, selectedColumn.llr, startIdx, szData, Settings::EXTENDED_LDPC_DIM);
            startIdx += selectedColumn.llr.size()/2; // TODO: Be carefull. Change this "2" hardcoded
        }
        llr_filestream.close();

        std::string ldpc_decoder_output_filename = "data.bin.tmp.col" + std::to_string(columnIdx);
        call_decode_ldpc(CodecConfig, CodecConfig.TMP_DATA_DIR_PATH[extent_idx], llr_filename,
                         ldpc_decoder_output_filename); //  281600 -> 256000

        std::string ldpc_encoder_input_filename = ldpc_decoder_output_filename;
        std::string ldpc_encoder_output_filename = ldpc_encoder_input_filename + ".ldpc";
        call_encode_ldpc(CodecConfig, CodecConfig.TMP_DATA_DIR_PATH[extent_idx], ldpc_encoder_input_filename,
                         ldpc_encoder_output_filename); // 256000 -> 281600

        std::uint32_t nBlocks = is_index_column ? 1 : 2;

        BitVector decodedData(nBlocks * Settings::FINAL_SIZE_LDPC_BLOCK);
        std::vector<BitVector> tmpBlocks(nBlocks, BitVector{Settings::FINAL_SIZE_LDPC_BLOCK});

        std::uint64_t j = 0;
        for (auto& block : tmpBlocks) {
            if (is_index_column) {
                block.RandomizeBits(FileConfig.RAND_SEED);
            } else {
                block.RandomizeBits(FileConfig.RAND_SEED + columnIdx * 2 - 1 + j);
            }
            j++;
        }
        {
            std::ifstream decodedDataLDPC(CodecConfig.TMP_DATA_DIR_PATH[extent_idx] + "/" + ldpc_encoder_output_filename);
            for (auto& block : tmpBlocks) {
                decodedDataLDPC >> block;
            }
        }
        fs::remove(fs::path(CodecConfig.TMP_DATA_DIR_PATH[extent_idx] + "/" + ldpc_encoder_output_filename));

        if (is_index_column) {
            for (auto& block : tmpBlocks) {
                selectedColumn.value = Serializer::SerializeVector(block,
                                                                   Settings::BITVALUE_SIZE - Settings::INDEX_LEN,
                                                                   Settings::INDEX_LEN);
            }
        } else {
            std::uint64_t offset = 0;
            for (auto& block : tmpBlocks) {
                auto tmpValues = Serializer::SerializeVector(block, Settings::BITVALUE_SIZE, 0 );
                std::copy(tmpValues.begin(), tmpValues.end(), selectedColumn.value.begin()+offset);
                offset += tmpValues.size();
            }
        }
        auto previousColumn = selectedColumn;

        encoder.Fill(selectedColumn.value, Settings::PREFIX_SIZE, Settings::NTS);
        encoder.EncodeValuesGroup();
        encoder.GetEncodingMotifs(selectedColumn.motif );

        std::uint64_t cnt = 0;
        for (size_t kMtf = 0; kMtf < previousColumn.N; kMtf++) {
            cnt += (selectedColumn.motif[kMtf].GetSequence() != previousColumn.motif[kMtf].GetSequence() ? 1 : 0);
        }
        LOG(debug) << "Motifs corrected: " << cnt <<std::endl;
        corrected_motifs[extent_idx][columnIdx] = cnt;
        /*
         * Replace the previously decoded motifs with the just corrected motifs
         * This would correct the prefix in the next round
         */
        decoder.ReplaceLastDecodedMotifs(selectedColumn.motif);
        if(columnIdx < (Settings::NUMBER_OF_MOTIFS - 1)) { /* Last motifs (column) don't need to be adjusted, as no other motif will follow */
            AdjustOligos(previousColumn, selectedColumn, selectedClusters, columnIdx);
        }
    }
}

void AdjustOligos(Column& previousColumn, Column& correctedColumn, std::vector<ClusterView>& clusters, size_t mtfIdx){

        for (size_t i = 0; i < previousColumn.Size(); i++) {
            if (previousColumn.isValid[i] && mtfIdx == Settings::IDX_OFFSET &&
                    previousColumn.motif[i].Compare(correctedColumn.motif[i]) != 0 ) {
                // TODO: Move reads between clusters only at the index 0
                correctedColumn.llr[i] = 0.0;
                correctedColumn.isValid[i] = false;
                clusters[i].Clear();
            } else if( previousColumn.isValid[i] ) {
                std::string correctedMotif = correctedColumn.motif[i].toString();
                {
                    uint32_t AlignmentWindow=4;
                    std::string subCorrectedMotif = correctedMotif.substr(AlignmentWindow*0, AlignmentWindow);
                    clusters[i].AdjustReads(subCorrectedMotif, AlignmentWindow);

                    subCorrectedMotif = correctedMotif.substr(AlignmentWindow*1, AlignmentWindow);
                    clusters[i].AdjustReads(subCorrectedMotif, AlignmentWindow);

                    subCorrectedMotif = correctedMotif.substr(AlignmentWindow*2, AlignmentWindow);
                    clusters[i].AdjustReads(subCorrectedMotif, AlignmentWindow);

                    subCorrectedMotif = correctedMotif.substr(AlignmentWindow*3, AlignmentWindow);
                    clusters[i].AdjustReads(subCorrectedMotif, AlignmentWindow);
                }
            }
        }
}

void WriteLLR(std::ofstream& fileStreamLLR, BitVector& blockLDPC, std::vector<float>& coefficient, std::uint32_t startIdx, std::uint32_t szData, std::uint32_t BLOCK_SIZE) {
    assert( blockLDPC.SizeInBits() >= BLOCK_SIZE );
    std::uint64_t cIdx = startIdx;
    float mult = coefficient[cIdx];
    float val = 0.0;
    for (std::uint64_t bIdx = 0; bIdx < BLOCK_SIZE; bIdx++) {
        if ( bIdx > 0 && (bIdx % szData) == 0) {
            cIdx++;
            mult = coefficient[cIdx];
        }
        bool bit = blockLDPC.GetBit(bIdx);
        if (!bit) {
            val = static_cast<float>( mult *
                          log((1.0 - Settings::EPS) / Settings::EPS));
        }else {
            val = static_cast<float>( mult *
                            (-log((1.0 - Settings::EPS) / Settings::EPS)));
        }
        fileStreamLLR << val << " ";
    }
    fileStreamLLR << std::endl;
}