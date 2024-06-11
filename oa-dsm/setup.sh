#!/bin/bash
#echo "[ INFO ] Setting OneAPI variables..."
#path_to_oneapi_vars=$(locate setvars.sh | head -n 1)
## ADD check if not empty the path
#source "$path_to_oneapi_vars"
#echo -e "\n"
echo "[ INFO ] Generating directory tree..."
mkdir -p data/extents
echo -e "[ INFO ] Directory tree ready\n\n"
echo -e "[ INFO ] Building LDPC codes repository...\n"
cd src/LDPC-codes || exit
make
cd ../
echo -e "[ INFO ] Build LDPC-codes completed.\n\n"
#echo "[ INFO ] Generating 30% LDPC matrix..."
#LDPC_path="./LDPC-codes/"
#LDPC_mat_path="./LDPC-codes/matrices"
#mkdir $LDPC_mat_path
## 30% redundancy (3,33) code
#$LDPC_path/make-ldpc $LDPC_mat_path/ldpc30.pchk 25600 281600 42 evenboth 3 no4cycle
#$LDPC_path/make-gen $LDPC_mat_path/ldpc30.pchk $LDPC_mat_path/ldpc30.gen dense
#$LDPC_path/extract_systematic $LDPC_mat_path/ldpc30.gen $LDPC_mat_path/ldpc30.systematic
#echo "[ INFO ] LDPC matrix ready.\n"
echo -e "[ INFO ] Building Codec...\n"
cd ../applications/XCoder || exit
rm -r build
mkdir build
cd build || exit
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
echo -e "[ INFO ] Build Codec completed.\n\n"

echo -e "[ INFO ] Building OneConsensus...\n"
cd ../../../applications/OneConsensus || exit
rm -r build
mkdir build
cd build || exit
cmake ..
make -j
cd ../../
echo -e "[ INFO ] Build OneConsensus completed.\n\n"
echo -e "[ INFO ] Build ReadsProcessingTools...\n"
cd ReadsProcessingTools || exit
rm -r build
mkdir build
cd build || exit
cmake ..
make -j
cd ../../../
echo -e "[ INFO ] Building ReadsProcessingTools completed...\n\n"
echo -e "[ INFO ] Write config file.\n"
bash ./config_template.sh -f data/config
echo -e "[ INFO ] Writing config file completed.\n"
echo "[ WARNING ] Some information need to be added to the config file at data/config"

