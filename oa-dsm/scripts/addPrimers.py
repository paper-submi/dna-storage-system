import sys
import argparse


def InitParser():
    parser = argparse.ArgumentParser(prog='AddPrimers',
                                     description='Add primers to all fasta files.')
    parser.add_argument('-i', dest='input', required=True,
                        help='prefix for file containing oligos per extent')
    parser.add_argument('-l', dest='left_primers_file', required=True,
                        help='file containing the left primer list')
    parser.add_argument('-r', dest='right_primers_file', required=True,
                        help='file containing the right primer list')
    parser.add_argument('-n', dest='n_extents', required=True, type=int,
                        help='number of extents to process')

    args = parser.parse_args()
    return args


def ImportPrimers(filename : str):
    primers = []
    file = open(filename, "r")
    while True:
        line = file.readline().rstrip()
        if line=="":
            break
        primers.append(line)

    return primers


if __name__ == '__main__':
    args = InitParser()

    input_file = args.input
    left_primers_file = args.left_primers_file
    right_primers_file = args.right_primers_file
    n = args.n_extents

    left_primers_list = ImportPrimers(filename=left_primers_file)
    right_primers_list = ImportPrimers(filename=right_primers_file)

    for extIdx in range(0,n):
        lIdx = int(int(extIdx) / len(right_primers_list))
        rIdx = int(extIdx) % len(right_primers_list)

        inFilePath = input_file+".EXTENT_"+str(extIdx)+".FASTA"
        outFilePath = inFilePath + ".primers"

        print("Appending primers ["+str(lIdx)+":"+left_primers_list[lIdx]+", "+str(rIdx)+":"+right_primers_list[rIdx]+ "] to " + inFilePath + "...")
        with open(inFilePath, "r") as input_F, open(outFilePath, "w") as output_F:
            for line in input_F:
                if line.startswith(">ref"):
                    output_F.write(line)
                else:
                    output_F.write(left_primers_list[lIdx] + line.rstrip() + right_primers_list[rIdx]+"\n")