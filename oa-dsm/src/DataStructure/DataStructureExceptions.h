#ifndef REFACTORINGCODEC_DATASTRUCTUREEXCEPTIONS_H
#define REFACTORINGCODEC_DATASTRUCTUREEXCEPTIONS_H

class bad_file_path : public std::exception {
public:
    virtual const char *what() const throw() {
        return "Cannot create the requested file";
    }
};

#endif //REFACTORINGCODEC_DATASTRUCTUREEXCEPTIONS_H
