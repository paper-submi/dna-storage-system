echo "Cleaning LDPC codes repository..."
cd Codec/LDPC-codes || exit
make clean
cd ../
echo "Cleaning LDPC-codes completed."
echo "Cleaning Codec..."
#mkdir /media/ssd/AllStringDir
cd ../
cd Codec || exit
rm -r build
echo "Cleaning Codec completed."
echo "Building Consensus..."
cd ../
cd OneConsensus || exit
rm -r build
echo "Cleaning Consensus completed."

