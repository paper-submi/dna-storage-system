#include <fstream>
#include <array>
#include <sys/wait.h> /* for wait */
#include <boost/filesystem.hpp>
#include "Utils.h"
#include "Logger.h"

using namespace std;
namespace fs = boost::filesystem;

void import_primers(vector<string> &primers, string &filename_primer) {
    ifstream input(filename_primer);
    if (!input.is_open()) {
        std::cerr << "Error in opening file " << filename_primer << std::endl;
        exit(-1);
    }
    string s;
    while (getline(input, s)) {
        primers.emplace_back(s);
    }
}

void exec_with_output( std::vector<char *> &args, string &output_buffer ) {
    const size_t BUFFER_SIZE=1024;
    std::array<char, BUFFER_SIZE> buff = {};

    int fd[2];
    int old_fd[3];
    if (pipe(fd)<0) {
        LOG(fatal) << "Cannot open the pipe";
        exit(-1);
    }


    old_fd[0] = !STDIN_FILENO;
    old_fd[1] = !STDOUT_FILENO;
    old_fd[2] = !STDERR_FILENO;

    old_fd[0] = dup(STDIN_FILENO);
    old_fd[1] = dup(STDOUT_FILENO);
    old_fd[2] = dup(STDERR_FILENO);

    int pid = fork();
    if (pid == 0){
        close(fd[0]);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);

        execv(args[0], &args.front());
        close(fd[1]);
        LOG(fatal) << "If here, there has been an error" << std::endl;
        exit(-1);
    }
    else{
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);

        while (read(fd[0], buff.data(), BUFFER_SIZE)){
            output_buffer.append(buff.data());
            memset(buff.data(), 0, BUFFER_SIZE);
        }

        waitpid(pid, nullptr, 0);
        close(fd[0]);
    }

    dup2(STDIN_FILENO, old_fd[0]);
    dup2(STDOUT_FILENO, old_fd[1]);
    dup2(STDERR_FILENO, old_fd[2]);
    close(old_fd[0]);
    close(old_fd[1]);
    close(old_fd[2]);
}

void print_statistics(std::vector<std::vector<size_t>>& motifs, ostream &out) {
    for(size_t i=0; i<motifs.size(); i++){
        out<<"Extent "<<i<<":"<<std::endl;
        for(size_t j=0; j<motifs.front().size(); j++){
            out<<"\tColumn "<<j <<":"<<std::endl;
            out<<"\t\tCorrected motifs"<<" : "<<motifs[i][j]<<std::endl;
        }
    }
}