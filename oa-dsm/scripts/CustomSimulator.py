import sys
import random
from optparse import OptionParser



INSERT = 0
DELETE = 1
SUBSTITUTION = 2

mapping={}
mapping['A']='T'
mapping['T']='A'
mapping['G']='C'
mapping['C']='G'

nucleotides = {}
nucleotides[0]='A'
nucleotides[1]='C'
nucleotides[2]='G'
nucleotides[3]='T'

def parse(argv):
    input_filepath = ''
    output_filepath = ''
    coverage=0
    error_rate=0.0
    length=0
    type=0


    parser = OptionParser()


    parser.add_option("-i", "--input", dest="input_filepath", type='string',
                      help="File containing oligos", metavar="INPUT")

    parser.add_option("-o", "--output", dest="output_filepath", type='string',
                  help="File containing reads", metavar="OUTPUT")


    parser.add_option("-c", "--cov", dest="coverage", type='int',
                      help="Coverage", metavar="COV")

    parser.add_option("-l", "--len", dest="length", type='int',
                      help="Reference length (of the encoding oligo)", metavar="LEN")

    parser.add_option("-e", "--err", dest="error_rate", type='int',
                      help="Error rate", metavar="RATE")

    parser.add_option("-t", "--type", dest="type", default=0, type='int',
                      help="Type of operation: 0 - Insertion+Deletion, 1 - Substitution", metavar="TYPE")

    (options, args) = parser.parse_args()

    if (options.type == None or options.input_filepath == None or options.output_filepath == None or options.coverage == None or options.error_rate == None):
        # print (parser.usage)
        parser.print_help()
        exit(0)


    input_filepath = options.input_filepath
    output_filepath = options.output_filepath
    coverage = options.coverage
    error_rate = options.error_rate
    length = options.length
    type = options.type
    return input_filepath, output_filepath, coverage, error_rate/100, length, type


def get_prob_string(motif_changed_idx):
    prob_string=""
    if motif_changed_idx != -1:
        for i in range(0,10):
            if i==motif_changed_idx:
                prob_string+="0 "
            else:
                prob_string+="10 "
    else:
        for i in range(0,10):
            prob_string+="10 "

    return prob_string


def read_dataset(filepath : str):
    dataset_file = open(filepath, "r")
    dataset = []
    while True:
        oligo = dataset_file.readline().rstrip()
        if oligo=="":
            break
        if oligo.startswith(">"):
            continue

        dataset.append(oligo)

    return dataset


def print_histogram(hist):
    max_symbols=20
    max_val=-1
    for val in hist.values():
        if val>max_val:
            max_val=val

    for i,val in sorted (hist.items()):
        print("%4d    " % i, end="")
        x=int(val/max_val*max_symbols)
        for i in range(x):
            print("*", end="")
        for i in range(max_symbols-x):
            print(" ", end="")
        print("%10d" % val)



def sim_insertion_deletion(error_rate : float, coverage : int, reference_length : int,  dataset : list):

    mean = int(error_rate * reference_length)
    dev = int(0.2 * mean)
    tot_number_of_operations=0
    reads_with_errors_at_beginning=0
    hist = {}
    #for i in range(200):
    #    hist[i]=0
    output_dataset=[]
    oligo_idx=0
    for oligo in dataset:
        output_dataset.append([])
        tmp_read = []
        for i in range(0,coverage):

            number_of_operation = int(random.gauss(mean, dev))
            tot_number_of_operations+=number_of_operation
            is_first=True
            tmp_read = list(oligo)
            for pos in range(number_of_operation):
                operation = int(random.uniform(0,2)) # Same uniform probability for INSERT or DELETE
                position = int(random.uniform(0,len(tmp_read))) # Same probability for any character in the string
                if(position in hist.keys()):
                    hist[position]+=1
                else:
                    hist[position]=1

                if(position<36 and is_first):
                    reads_with_errors_at_beginning+=1
                    is_first=False

                if operation == INSERT:
                    nt = int(random.uniform(0,4))
                    tmp_read.insert( position, nucleotides[nt] )
                elif operation == DELETE:
                    del tmp_read[ position ]

            # Make the string of the right length
            '''if len(tmp_read)>len(oligo):
                tmp_read=tmp_read[0:len(oligo)]
            elif len(tmp_read)<len(oligo):
                to_add = len(oligo) - len(tmp_read)

                for i in range(to_add):
                    nt = int(random.uniform(0,4))
                    tmp_read.append( nucleotides[nt] )
            '''
            read=""
            read = "".join(tmp_read)
            output_dataset[oligo_idx].append(read)
        oligo_idx+=1

    print("Inserted "+str(tot_number_of_operations)+" errors")
    print("Reads with errors in first 36 nts:  "+str(reads_with_errors_at_beginning))
    print_histogram(hist)
    return output_dataset

def sim_all(error_rate : float, coverage : int, reference_length : int,  dataset : list):

    mean = int(error_rate * reference_length)
    dev = int(0.2 * mean)
    tot_number_of_operations=0
    reads_with_errors_at_beginning=0
    hist = {}
    #for i in range(200):
    #    hist[i]=0
    output_dataset=[]
    oligo_idx=0
    for oligo in dataset:
        output_dataset.append([])
        tmp_read = []
        for i in range(0,coverage):

            number_of_operation = int(random.gauss(mean, dev))
            tot_number_of_operations+=number_of_operation
            is_first=True
            tmp_read = list(oligo)
            for pos in range(number_of_operation):
                operation = int(random.uniform(0,3)) # Same uniform probability for INSERT or DELETE
                position = int(random.uniform(0,len(tmp_read))) # Same probability for any character in the string
                if(position in hist.keys()):
                    hist[position]+=1
                else:
                    hist[position]=1

                if(position<36 and is_first):
                    reads_with_errors_at_beginning+=1
                    is_first=False

                if operation == INSERT:
                    nt = int(random.uniform(0,4))
                    tmp_read.insert( position, nucleotides[nt] )
                elif operation == DELETE:
                    del tmp_read[ position ]
                elif operation == SUBSTITUTION:
                    nt = nucleotides[ int(random.uniform(0,4)) ]
                    while nt==tmp_read[position]:
                        nt = nucleotides[ int(random.uniform(0,4)) ]
                    tmp_read[position]=nt

            # Make the string of the right length
            '''if len(tmp_read)>len(oligo):
                tmp_read=tmp_read[0:len(oligo)]
            elif len(tmp_read)<len(oligo):
                to_add = len(oligo) - len(tmp_read)

                for i in range(to_add):
                    nt = int(random.uniform(0,4))
                    tmp_read.append( nucleotides[nt] )
            '''
            read=""
            read = "".join(tmp_read)
            output_dataset[oligo_idx].append(read)
        oligo_idx+=1

    print("Inserted "+str(tot_number_of_operations)+" errors")
    print("Reads with errors in first 36 nts:  "+str(reads_with_errors_at_beginning))
    print_histogram(hist)
    return output_dataset


def sim_substitution_only(error_rate : float, coverage : int, reference_length : int,  dataset : list):

    mean = int(error_rate * reference_length)
    dev = int(0.2 * mean)
    oligo_idx=0
    for oligo in dataset:
        tmp_read = []
        for i in range(0,coverage):

            number_of_operation = int(random.gauss(mean, dev))

            tmp_read = list(oligo)
            for pos in range(number_of_operation):

                position = int(random.uniform(0,len(tmp_read))) # Same probability for any character in the string

                nt = nucleotides[ int(random.uniform(0,4)) ]
                while nt==tmp_read[position]:
                    nt = nucleotides[ int(random.uniform(0,4)) ]

                tmp_read[position]=nt

            # Make the string of the right length
            read=""
            read = "".join(tmp_read)
            output_dataset[oligo_idx].append(read)
        oligo_idx+=1
    return output_dataset

if __name__ == '__main__':

    reference_length = 0

    coverage = 0
    error_rate = 0
    type = 0

    input_filepath=""#"./data/tpchdb.7z.FASTA"
    output_filepath=""

    input_filepath, output_filepath, coverage, error_rate, reference_length, type = parse(sys.argv[1:])

    dataset = read_dataset(input_filepath)
    output_dataset=[]

    if type==0:
        output_dataset = sim_insertion_deletion(error_rate, coverage, reference_length, dataset)
    elif type==1:
        output_dataset = sim_substitution_only(error_rate, coverage, reference_length, dataset)
    elif type==2:
        output_dataset = sim_all(error_rate, coverage, reference_length, dataset)
    else:
        exit(-1)

    output_file = open(output_filepath, "w")

    for cluster in output_dataset:
        output_file.write(str(len(cluster))+"\n")
        for read in cluster:
            output_file.write(read+"\n")
