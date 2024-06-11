#!/bin/bash

show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-i FILEPATH] [-s CENTROID_PATH] [-o OUTPUT_DIR] [-3 RIGHT_PRIMER ]

-h                  display this help and exit.
-i FILEPATH         filename of the encoded file
-s CENTROID_PATH    path to the centroids selected in decoding phase
-o OUTPUT_DIR       output directory in which store extents/columns motifs
-3 RIGHT_PRIMER     right primer path
EOF
}

OPTIND=1 # Reset in case getopts has been used previously in the shell.


fasta_path=""
selected_centroids_dir=""
output_dir=""

while getopts "hi:s:o:3:" opt; do
  case "$opt" in
    h)
      show_help
      exit 0
      ;;
    i)
      fasta_path=$OPTARG
      echo "[ INFO ] Reading FASTAs for $fasta_path file"
      ;;
    s)
      selected_centroids_dir=$OPTARG
      echo "[ INFO ] Reading selected centroids from $selected_centroids_dir/seleceted_centroids* dir"
      ;;
    o)
      output_dir=$OPTARG
      echo "[ INFO ] Writing results in $output_dir"
      ;;
    3)
      right_primers_path=$OPTARG
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


rm -r "$output_dir"
mkdir "$output_dir"

N_EXTENTS=$(ls -l $selected_centroids_dir/selected_centroids* | wc -l)

N_MOTIFS=10

for extent_idx in $(seq 0 $(($N_EXTENTS-1)));
do
  for col in $(seq 0 $(($N_MOTIFS-1))); do
    output_file="$output_dir/extent_$extent_idx.column_$col"
    input_file=$selected_centroids_dir/selected_centroids$extent_idx
    cat $input_file | awk -v i=$col '{
        if($1=="INVALID"){
          print "NOT_VALID";
        }else{
          print substr($1, i*16+1, 16);
        }
    }' > $output_file
    done
    #extent_idx=$(($extent_idx+1))
done

N=$( cat $right_primers_path | wc -l )

for extent_idx in $(seq 0 $(($N_EXTENTS-1))); do
  L_idx=$(($extent_idx / N))
  R_idx=$(($extent_idx % N))
  echo "EXTENT $extent_idx"
  fasta="$fasta_path.$L_idx""_$R_idx.FASTA"
  for col in $(seq 0 $(($N_MOTIFS-1))); do
    output_file="$output_dir/extent_$extent_idx.column_$col"
    TOT=$(cat $fasta | grep -v "ref" | wc -l)
    NOT_MATCHING=$(cat $fasta | grep -v "ref" | grep -o -f $output_file | sort | uniq | wc -l)
    D=$((TOT - NOT_MATCHING))
    echo -n "$D "
  done
  echo ""
  #extent_idx=$(($extent_idx+1))
done