#!/bin/bash

########################
#    DECODER SCRIPT    #
########################

# INPUT: Reads dataset coming from simulator/sequencing

# 1. Run GroupReads in order to separate out reads based on primers
# 3. Copy the results in the extents directory
# 4. Apply OneConsensus in order to cluster and get consensus among the reads
# 2. Run PrimerRemoval in order to extract only the payload (the reference) from the centroids dataset
# 5. Run decoder in order to build back the original file

show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-r READS_FILEPATH] [-d DATA_PATH] [-5 LEFT_PRIMER_FILE] [-3 RIGHT_PRIMER_FILE] [-l LENGTH] [-p N_PTS] [-e ED]

-h                  display this help and exit.
-r READS_FILEPATH   dataset containing reads to process.
-d DATA_PATH        path to the data directory, containing the extents directory tree [default ./data]
-5 L_PRIMER_FILE    reft primer file.
-3 R_PRIMER_FILE    right primer file.
-l N                length of the reference oligos.
-p NUM_OF_PTS       number of point in order to consider valid a cluster
-e ED               edit distance to use for align the primers

EOF
}

OPTIND=1 # Reset in case getopts has been used previously in the shell.


reads_path=""
left_primers_path=""
right_primers_path=""
length=""
number_of_pts=""
edit_distance=""
data_path="./data"

while getopts "hr:d:5:3:l:p:e:" opt; do
  case "$opt" in
    h)
      show_help
      exit 0
      ;;
    r)
      reads_path=$OPTARG
      echo "[ INFO ] Reading reads dataset from $reads_path"
      ;;
    d)
      data_path=$OPTARG
      ;;
    5)
      left_primers_path=$OPTARG
      ;;
    3)
      right_primers_path=$OPTARG
      ;;
    l)
      length=$OPTARG
      ;;
    p)
      number_of_pts=$OPTARG
      ;;
    e)
      edit_distance=$OPTARG
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

if [ "$reads_path" = "" ] || [ "$data_path" = "" ] || [ "$left_primers_path" = "" ] || [ "$right_primers_path" = "" ] || [ "$length" = "" ] || [ "$edit_distance" = "" ] || [ "$number_of_pts" = "" ]; then
  show_help
  exit 255
fi


N_EXTENTS=$(cat $data_path/params | grep  "N_EXTENTS =" | grep -o -E "[0-9]+")

if ! [[ $N_EXTENTS =~ ^[0-9]+$ ]] ; then
   echo "error: N_EXTENTS is not a number" >&2; exit 1
fi

rm -r $data_path/extents/ext_*/

for ext_idx in $(seq 0 $(($N_EXTENTS-1))); do
  mkdir "$data_path/extents/ext_$ext_idx"
done

ext_idx=0

echo "[ INFO ] Grouping reads based on left/right primers\n"

./applications/ReadsProcessingTools/build/GroupReads -i "$reads_path" -5 "$left_primers_path" -3 "$right_primers_path" -e "$edit_distance" -l $length


echo "[ INFO ] Removing primers\n"

ext_idx=0
extent_dir=""

n_right_primers=$(cat $right_primers_path | wc -l)

for ext_idx in $(seq 0 $(($N_EXTENTS-1))); do
  L_idx=$(($ext_idx / $n_right_primers))
  R_idx=$(($ext_idx % $n_right_primers))

  input_filename="L""$L_idx""_R""$R_idx"
  extent_dir="$data_path/extents/ext_$ext_idx"
  ./applications/ReadsProcessingTools/build/RemovePrimers -i "$input_filename" -o "$extent_dir/$input_filename.trimmed"  -5 "$left_primers_path" -3 "$right_primers_path"  -l $(($length)) -e "$edit_distance"
  rm $input_filename
done
rm L*

echo "[ INFO ] Running clustering and consensus\n"

extent_dir=""

#n_right_primers=$(cat $right_primers_path | wc -l)

for ext_idx in $(seq 0 $(($N_EXTENTS-1))); do
  L_idx=$(($ext_idx / $n_right_primers))
  R_idx=$(($ext_idx % $n_right_primers))

  input_filename="L""$L_idx""_R""$R_idx.trimmed"
  extent_dir="$data_path/extents/ext_$ext_idx"
  number_of_reads=$( cat "$extent_dir/$input_filename" | wc -l )

  if [ $number_of_reads -gt 0 ]; then
   ./applications/OneConsensus/build/oneconsensus_shortread -r "$extent_dir/$input_filename" -o 0 -l $(($length))
    cat clusters_thread* >  $extent_dir"/clusters"

    rm clusters_thread*
    #rm log_thread*
    rm experiment-parameters-*
    rm time-report*
  fi
    rm $extent_dir/$input_filename
done

echo "[ INFO ] Decoding oligos\n"

./applications/XCoder/build/decoder -p $data_path/params -g $data_path/config