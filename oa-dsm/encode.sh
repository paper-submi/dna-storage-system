#!/bin/bash

########################
#    ENCODER SCRIPT    #
########################

# INPUT: Reads dataset coming from simulator/sequencing

# 1. Run GroupReads in order to separate out reads based on primers
# 3. Copy the results in the extents directory
# 4. Apply OneConsensus in order to cluster and get consensus among the reads
# 2. Run PrimerRemoval in order to extract only the payload (the reference) from the centroids dataset
# 5. Run decoder in order to build back the original file

show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-f FILE_PATH] [-g CONFIG_PATH] [-p PARAM_FILEPATH]

-h                  display this help and exit.
-f FILE_PATH        path to the file to encode
-g CONFIG_PATH  path to the configuration file (config) of the encoder
-p PARAM_FILEPATH   Path to the param file which will be created during the encoding
EOF
}

OPTIND=1 # Reset in case getopts has been used previously in the shell.


FILE_PATH=""
CONFIG_PATH=""
PARAM_PATH=""

while getopts "hf:g:p:" opt; do
  case "$opt" in
    h)
      show_help
      exit 0
      ;;
    f)
      FILE_PATH=$OPTARG
      echo "[ INFO ] Reading reads dataset from $reads_path"
      ;;
    g)
      CONFIG_PATH=$OPTARG
      ;;
    p)
      PARAM_PATH=$OPTARG
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
  exit 255
fi

if [ "$FILE_PATH" = "" ] || [ "$CONFIG_PATH" = "" ] || [ "$PARAM_PATH" = "" ]; then
  show_help
  exit 255
fi

if [ -f "data/config" ];
then
    ldpc_prefix=$(cat "$CONFIG_PATH" | grep LDPC_PREFIX | awk '{print $3}')
else
    echo "[ FATAL ] Configuration file missing at data/config"
    exit 255
fi

if [ -f "$ldpc_prefix.gen" ] && [ -f "$ldpc_prefix.pchk" ] && [ -f "$ldpc_prefix.systematic" ];
then
./applications/XCoder/build/encoder -f "$FILE_PATH" -p "$PARAM_PATH" -g "$CONFIG_PATH"
else
  echo "[ FATAL ] LDPC matrices are missing at $ldpc_prefix. Cannot encode $FILE_PATH"
fi

#N_EXTENTS=$(cat $PARAM_PATH | grep N_EXTENTS | awk '{print $3}')

#rm "$FILE_PATH.FASTA"

#for extent_idx in $(seq 0 $(($N_EXTENTS-1)));do
 # cat "$FILE_PATH.$extent_idx""_"$extent_idx.FASTA >> "$FILE_PATH.FASTA"
#done

