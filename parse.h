#pragma once
#include "branchless-utf8/utf8.h"
#include "SSO-23/include/string.hpp"
#include "kspp/ks.h"
#include "unistd.h"
#include "zlib.h"

namespace dnlp {
using sstring = sso23::string; // pstring for sso string.

#if 0
template<typename SourceType>
struct CharEmitter;

struct ASCIIEmitter {
    void assign(std::FILE *fp) {

    }
};

template<> struct CharEmitter<std::FILE*> {
    std::FILE  *fp_;
    uint8_t owns_:1;
    CharEmitter(std::FILE *fp, bool owns=true): fp_(fp), owns_(owns) {}
    CharEmitter(const char *path, const char *fmt): fp_(std::fopen(path, fmt)), owns_(true) {
        if(!fp_) throw std::runtime_error(ks::sprintf("Could not open file at %s\n", path).data());
    }
    ~CharEmitter() {
        if(owns_) std::fclose(fp_);
    }
    int getc() {
        return std::fgetc(fp_);
    }
    template<typename Functor>
    void for_each_char(const Functor &func) {
        int c;
        while((c = this->getc()) != EOF) func(c);
    }
};

struct ASCIIEmitter {
    void assign(std::FILE *fp) {

    }
};
#endif

/*
   Progress!
    still needs:
        setting strings from char *p/len
        string splitting policy
        put the strings together
        get them hashes somehow
        Pass the ngrams to someone else
*/

template<typename SymbolType=char>
class NGrammer {
    union Handle {
        std::FILE *fp_;
        gzFile *gzfp_;
        Handle() {fp_ = nullptr;}
    };
    union CPtr {
        char      *nc_;
        const char *c_;
    };
    Handle h_; // Does not own either!
    CPtr  cp_; // Contains a char pointer of either const or nonconst
    uint32_t            l_;
    uint8_t mutable_seq_:1;
    uint16_t         n_:15;
    circ::deque<sso23::basic_string<SymbolType>> deque_;
public:
    NGrammer(unsigned n, bool mutable_seq=true): mutable_seq_(mutable_seq), n_(n), deque_(n) {
        if(n > n_) throw std::runtime_error(ks::sprintf("NGrammer can only handle n of up to %zu", size_t(1 << 15)).data());
    }
    void assign(char *s, ssize_t l) {
        l_ = l;
        cp_.nc_ = s;
        mutable_seq_ = true;
    }
    void assign(const char *s, ssize_t l) {
        l_ = l;
        cp_.c_ = s;
        mutable_seq_ = false;
    }
};

}
