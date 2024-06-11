#!/bin/bash

################################
#    Reads simulator script    #
################################

# 1. Compute reads in FASTQ format starting from the encoding oligos in FASTA format
# 2. Filter only the forward reads
# 3. Save the reads as set of strings in the  reads_dataset file

show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-i INPUT_FILE] [-o OUTPUT_FILE] [-c COV] [-m MIN_LENGTH] [-M MAX_LENGTH ]
Do stuff with FILE and write the result to standard output. With no FILE
or when FILE is -, read standard input.

-h                  display this help and exit.
-i INPUT_FILE       FASTA file containgin the encoding oligos.
-o OUTPUT_FILE      FASTQ file containing the simulated reads.
-c COV              coverage to use in the simulation.
-m MIN_LENGTH       min length of the simulated reads.
-M MAX_LENGTH       max length of the simulated reads.
EOF
}


OPTIND=1 # Reset in case getopts has been used previously in the shell.

while getopts "hs:i:o:m:M:c:" opt; do
  case "$opt" in
    h)
      show_help
      exit 0
      ;;
    s)
      simulator=$OPTARG
      ;;
    i)
      simulator_input=$OPTARG
      echo "[ INFO ] Reading encoding oligos from $simulator_input"
      ;;
    o)
      simulator_output=$OPTARG
      ;;
    m)
      minlen=$OPTARG
      ;;
    M)
      maxlen=$OPTARG
      ;;
    c)
      cov=$OPTARG
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

out=stdout

sim_path=/BBMap

if [ "$maxlen" = "" ] || [ "$minlen" = "" ] || [ "$cov" = "" ] || [ "$simulator_input" = "" ] || [ "$simulator_output" = "" ]; then
  show_help
  exit 1
fi
"$sim_path"/randomreads.sh minlength=$minlen maxlength=$maxlen ref=$simulator_input adderrors=t coverage=$cov out=$out 2>log_stderr_simulator > $simulator_output

cat log_stderr_simulator #It's the filename, not a variable
rm  log_stderr_simulator

if [ -s "$simulator_output" ]
then
 echo "[ INFO ] Reads written in $simulator_output"
 exit 0
else
 exit 255
fi

