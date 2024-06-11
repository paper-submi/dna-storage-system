#!/bin/bash
show_help() {
cat << EOF
Usage: ${0##*/} [-h]  [-f CONFIG_FILEPATH]
Do stuff with FILE and write the result to standard output. With no FILE
or when FILE is -, read standard input.

-h                  display this help and exit.
-f CONFIG_FILEPATH  config filepath
EOF
}

OPTIND=1 # Reset in case getopts has been used previously in the shell.

config_filepath=""

while getopts "hf:" opt; do
  case "$opt" in
    h)
      show_help
      exit 0
      ;;
    f)
      config_filepath=$OPTARG
      ;;
    \?)
      echo "[ ERROR ] Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "[ ERROR ] Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

if [ $OPTIND -eq 1 ]; then
  show_help
  exit
fi

rm $config_filepath

REPOSITORY_PATH=$(pwd)
echo "# Change this line to point to the right matrix when you change the redundancy
# Note that to change redundancy factor, you need to:
#   change the LDPC_ALPHA value in Constants.cpp file;
#   ricompile;
#   update the matrix path here.

LDPC_PREFIX = $REPOSITORY_PATH/src/LDPC-codes/matrices/ldpc10
LDPC_MAX_ITER = 100

# Note that if tables are not found at TABLE_PATH, they will be generated at the moment.
# This wil require ~100GB and a lot of time (~hours).

TABLE_PATH = <PATH TO ENCODING TABLES DIR>

# Clusters filename is the prefix to use for the file containing:
#   centroids;
#   number of points;
#   motifs probability.
# For example, if this path is set as <clusters>, the program will look for
#   clusters, clusters.pts and clusters.prob in the extent directory.
# All scripts in this repository assume <clusters> as filename.

# DECODER_INPUT_PATH = <CLUSTERS FILENAME ONLY>
DECODER_INPUT_PATH = clusters

LEFT_PRIMERS_PATH = $REPOSITORY_PATH/data/left_primer
RIGHT_PRIMERS_PATH = $REPOSITORY_PATH/data/right_primer

DATA_DIR_PATH = $REPOSITORY_PATH/data
EXTENTS_DATA_DIR_PATH = $REPOSITORY_PATH/data/extents
TMP_ENCODER_DATA_PATH = $REPOSITORY_PATH/tmp_data
LDPC_ENCODE_PATH = $REPOSITORY_PATH/src/LDPC-codes/encode
LDPC_DECODE_PATH = $REPOSITORY_PATH/src/LDPC-codes/decode
LDPC_EXTRACT_PATH = $REPOSITORY_PATH/src/LDPC-codes/extract" >> $config_filepath

echo "Written: "
cat $config_filepath