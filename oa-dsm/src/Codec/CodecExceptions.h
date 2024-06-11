#ifndef REFACTORINGCODEC_CODECEXCEPTIONS_H
#define REFACTORINGCODEC_CODECEXCEPTIONS_H


class bad_oligo : public std::exception {
public:
    virtual const char *what() const throw() {
        return "Oligo too big. Encoding failed. Exit.";
    }
};

class bad_encoding_table : public std::exception {
public:
    virtual const char *what() const throw() {
        return "Oligo too big. Encoding failed. Exit.";
    }
};

class tables_not_found : public std::exception {
public:
    virtual const char *what() const throw() {
        return "Tables not found at the specified path.";
    }
};


class bad_prefix : public std::exception {
public:
    virtual const char *what() const throw() {
        return "Wrong prefix used to access the motif tables";
    }
};

class bad_value : public std::exception {
public:
    virtual const char *what() const throw() {
        return "It is not possible to encode the value";
    }
};

class invalid_data_size : public std::exception {
public:
    virtual const char *what() const throw() {
        return "Data size is not multiple of value size";
    }
};




#endif //REFACTORINGCODEC_CODECEXCEPTIONS_H
