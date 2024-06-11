#include <sstream>
#include "Motif.h"


Motif::Motif(motif_t seq, std::uint32_t len) : length(len), sequence(seq) {}


Motif::Motif(std::string_view motif, std::uint32_t szMotif) {
    /* It can't be -1
    if(szMotif == -1){
        szMotif = motif.size();
    }*/
    if (szMotif > MAX_NTS || motif.size() > szMotif) {
        throw bad_length();
    }

    motif_t seq = 0;
    nucleotide_t nt;
    for (char c : motif) {
        nt = NtsToVal(c);
        if (nt == 255) {
            throw bad_nucleotide();
        }
        seq = (seq << 2) | motif_t(nt);
    }

    std::uint32_t toAdd = szMotif - motif.size();
    seq = (seq << 2*toAdd);

    this->length = szMotif;
    this->sequence = seq;
}

Motif::Motif(std::string_view motif) {
    std::uint32_t szMotif = motif.size();
    if (szMotif > MAX_NTS || motif.size() > szMotif) {
        throw bad_length();
    }

    motif_t seq = 0;
    nucleotide_t nt;
    for (char c : motif) {
        nt = NtsToVal(c);
        if (nt == 255) {
            throw bad_nucleotide();
        }
        seq = (seq << 2) | motif_t(nt);
    }

    std::uint32_t toAdd = szMotif - motif.size();
    seq = (seq << 2*toAdd);

    this->length = szMotif;
    this->sequence = seq;
}


std::string Motif::toString() const {
    std::string motif;
    for (std::uint64_t i = 0; i < this->length; i++) {
        motif += ValToNts(this->At(i));
    }
    return motif;
}

bool Motif::Check() const {
    size_t motifLength = this->length;
    if (motifLength <= 0) {
        return false;
    }

    std::int32_t freqNts[4] = {0};
    nucleotide_t firstNt = this->At(0);
    freqNts[firstNt]++;

    size_t n = 1;
    for (size_t i = 1; i < motifLength; i++) {

        nucleotide_t currentNt = this->At(i);
        freqNts[currentNt]++;

        if (currentNt == firstNt) {
            n++;
            if (n > 2) {
                return false;
            } else if (currentNt == Nucleotides::G && n > 2) { // Useless???
                return false;
            }
        } else {
            firstNt = currentNt;
            n = 1;
        }
    }

    float ratioCG = (float) (freqNts[Nucleotides::C] + freqNts[Nucleotides::G]) / (float) motifLength;
    if (ratioCG < 0.3 || ratioCG > 0.7) {
        return false;
    }
    return true;
}

nucleotide_t Motif::At(std::uint32_t idx) const {
    return this->sequence >> (2 * (this->length - idx - 1)) & 0x3;
}

char Motif::operator[](std::uint32_t idx) const {
    return ValToNts(this->At(idx));
}

bool Motif::Next() {
    motif_t max = (static_cast<motif_t>(1) << (2 * this->length)) - 1;
    if (this->length == MAX_NTS || this->sequence == max) {
        return false;
    }
    this->sequence++;
    return true;
}

std::string Motif::GetLastChars(std::uint32_t n) const {
    try {
        auto slicedMotif = this->Slice(length - n, n);
        return slicedMotif.toString();
    }catch (bad_index &e){
        throw bad_length();
    }
}

Motif Motif::Slice(std::int64_t start, std::int64_t n) const {

    if (start < 0 || start > this->length || start + n > this->length){
        throw bad_index();
    }
    if(n < -1 || n == 0){
        throw bad_index();
    }

    if (n == -1) {
        n = this->length - start;
    }

    std::uint32_t szMotif = n;
    std::uint32_t end = start + n;

    auto omask = static_cast<motif_t>((static_cast<motif_t>(1) << (2 * szMotif)) - 1);
    Motif o{ static_cast<uint32_t>((this->sequence >> (2 * (this->length - end))) & omask), szMotif};
    return o;
}

bool Motif::Append(Motif &other) {

    if (this->length + other.length > MAX_NTS) {
        throw bad_length();
    }

    this->length += other.length;
    for (std::uint64_t i = 0; i < other.length; i++) {
        nucleotide_t currentNt = other.At(i);
        this->sequence = static_cast<motif_t>(this->sequence << 2) | static_cast<motif_t>(currentNt);
    }
    return true;
}


motif_t Motif::GetSequence() const{
    return this->sequence;
}

std::uint32_t Motif::GetLength() const{
    return this->length;
}


void Motif::SetAt(std::uint32_t idx, std::uint8_t val) {
    if(idx >= this->length){
        throw bad_index();
    }
    std::uint32_t pos = 2 * (this->length - idx - 1);
    this->sequence &= ~(0x3 << pos);
    this->sequence |= static_cast<motif_t>( (val & 0x3) ) << pos;
}

int Motif::Compare(Motif &other) const {

    if (this->length < other.length) {
        return -1;
    } else if (this->length > other.length) {
        return 1;
    }

    if (this->sequence < other.sequence) {
        return -1;
    } else if (this->sequence > other.sequence) {
        return 1;
    }

    size_t olen = other.length;

    if (this->length < olen) {
        return -1;
    } else if (this->length > olen) {
        return 1;
    }

    for (size_t i = 0; i < this->length; i++) {
        int n = this->At(i) - other.At(i);
        if (n < 0) {
            return -1;
        } else if (n > 0) {
            return 1;
        }
    }

    return 0;
}


std::string Oligo::toString() {
    std::ostringstream ss;
    for (const Motif& motif : this->oligo) {
        ss << motif;
    }
    return ss.str();
}

void Oligo::SetOligo(std::string_view other, std::uint32_t szMotif){

    this->oligo.clear();
    std::uint32_t nMotifs = ((other.size() + szMotif - 1) / szMotif * szMotif)/szMotif;

    this->oligo.resize( nMotifs, Motif{szMotif, 0} );

    /* ExtractBits the oligo according to motif length */
    std::uint32_t start=0;
    for( std::uint32_t mIdx = 0; mIdx < oligo.size(); mIdx++ ){
        std::uint64_t n = (start + szMotif) < other.size() ? szMotif : std::string_view::npos;
        auto mtfView = other.substr(start, n);
        Motif mtf(mtfView);
        this->oligo[mIdx] = mtf;
        start+=szMotif;
    }
}

const Motif &Oligo::operator[](std::uint64_t mIdx) {
    return this->oligo[mIdx];
}

std::uint32_t Oligo::GetMotifLength(){
    return this->oligo.empty() ? 0 : this->oligo.front().GetLength();
}

void Oligo::AppendMotif(Motif &motif) {
    oligo.emplace_back(motif);
}