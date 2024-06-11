#ifndef ALLSTRINGS_OLIGO_H
#define ALLSTRINGS_OLIGO_H

#include <vector>
#include <iostream>
#include <string>

class bad_length : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Motif too long.";
    }
};

class bad_index : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Index out of bound";
    }
};

class bad_nucleotide : public std::exception{
public:
    virtual const char *what() const throw() {
        return "Wrong nucleotide: cannot cast string to sequence";
    }
};

using short_motif_t = std::uint32_t;
using motif_t = std::uint64_t;
using nucleotide_t = std::uint8_t;
const std::uint32_t MAX_NTS = 32;

enum Nucleotides {
    A, C, G, T
};


inline char ValToNts(std::uint8_t v) {
    if (v == 0) {
        return 'A';
    }
    if (v == 1) {
        return 'C';
    }
    if (v == 2) {
        return 'G';
    }
    if (v == 3) {
        return 'T';
    }
    return 'Z'; // ERROR
}

inline nucleotide_t NtsToVal(char c) {
    if (c == 'A') {
        return 0;
    }
    if (c == 'C') {
        return 1;
    }
    if (c == 'G') {
        return 2;
    }
    if (c == 'T') {
        return 3;
    }
    if (c == 'N') {
        return 4;
    }
    return 255; // ERROR
}

class Motif {
    std::uint32_t length;
    motif_t sequence;

public:
    Motif( motif_t seq, std::uint32_t len);
    Motif(std::string_view motif);
    explicit Motif(std::string_view motif, std::uint32_t szMotif);

    [[nodiscard]] motif_t GetSequence() const;
    [[nodiscard]] std::uint32_t GetLength() const;

    [[nodiscard]] nucleotide_t At(uint32_t idx) const;
    void SetAt(std::uint32_t pos, std::uint8_t val);
    [[nodiscard]] char operator[](std::uint32_t idx) const;

    bool Next();

    [[nodiscard]] bool Check() const;

    std::int32_t Compare(Motif &other) const;

    [[nodiscard]] Motif Slice(std::int64_t start, std::int64_t n = - 1) const;
    bool Append(Motif &other);
    [[nodiscard]] std::string GetLastChars(std::uint32_t n) const;

    [[nodiscard]] std::string toString() const;

    friend std::ostream &operator<<(std::ostream &os, const Motif &ol) {
        os << ol.toString();
        return os;
    }
};

class Oligo {
    std::vector<Motif> oligo;
public:
    [[nodiscard]] std::string toString() ;
    void SetOligo(std::string_view other, std::uint32_t szMotif);
    void AppendMotif(Motif &motif);
    const Motif& operator[](std::uint64_t mIdx);
    std::uint32_t GetMotifLength();
};

#endif //ALLSTRINGS_OLIGO_H
