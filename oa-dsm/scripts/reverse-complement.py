import sys
import re

def rcomplement(oligo):
    complement_map = {'A':'T','G':'C','T':'A','C':'G'}
    return ''.join([complement_map[x] for x in oligo[::-1]])


if __name__ == '__main__':


    while True:
        s=""
        #s = rcomplement("ACGAAGT")

        line1 = sys.stdin.readline()#.rstrip()

        if(not line1):
            break

        line1 = line1.rstrip()
        line2 = sys.stdin.readline()
        line3 = sys.stdin.readline().rstrip()
        line4 = sys.stdin.readline().rstrip()

        read = line2.rstrip()

        if("_-_" in line1):
            #print("NNNNNNN"+read+"NNNNNN")
            s = rcomplement(read)
            #print("NNNNNNN"+s+"NNNNNN")
        else:
            s = read

        print(line1)
        print(s)
        print(line3)
        print(line4)
