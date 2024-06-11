#!/bin/bash

show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-f FASTA_PREFIX] [-n NUM_EXTENTS]

EOF
}

OPTIND=1 # Reset in case getopts has been used previously in the shell.


fasta_prefix=""
num_extents=""

while getopts "hf:n:" opt; do
  case "$opt" in
    h)
      show_help
      exit 0
      ;;
    f)
      fasta_prefix=$OPTARG
      echo "[ INFO ] Reading reads dataset from $reads_path"
      ;;
    n)
      num_extents=$OPTARG
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


rm "$fasta_prefix.FASTA"

for ext_idx in $(seq 0 $(($num_extents-1))); do
  L_idx=$ext_idx
  R_idx=$ext_idx
  file="$fasta_prefix.$L_idx""_$R_idx.FASTA"
  cat "$file" >> "$fasta_prefix.FASTA"

done