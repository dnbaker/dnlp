#pragma once
#include <cstring>
#include <fstream>
#include "branchless-utf8/utf8.h"
#include "SSO-23/include/string.hpp"
#include "kspp/ks.h"
#include "clhash/include/clhash.h"
#include "circularqueue/circular_buffer.h"
#include "valptr/valptr.h"
#include "unistd.h"
#include "logutil.h"
#include "zlib.h"

namespace dnlp {
using sstring = sso23::string; // pstring for sso string.

static std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

#if 0
template<typename SourceType>
struct CharEmitter;

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

template<typename SymbolType>
using NGramType = circ::deque<sso23::basic_string<SymbolType>>;
template<typename SymbolType>
using GramType = sso23::basic_string<SymbolType>;

template<typename HashStruct>
struct NGramHasherBase {
    HashStruct hs_;
    template<typename... Args>
    NGramHasherBase(Args &&... args): hs_(std::forward<Args>(args)...) {
        std::fprintf(stderr, "Made %s\n", __PRETTY_FUNCTION__);
    }
    template<typename SymbolType>
    uint64_t operator()(const NGramType<SymbolType> &ngram) const {
        auto ind = ngram.start();
        const auto end = ngram.stop();
        if(__builtin_expect(ind == end), 0) return 0;
        auto mask = ngram.mask();
        auto data = ngram.data();
        uint64_t ret = hs_(data[ind++]), tmp;
        ind &= mask;
        while(ind != end) {
            GramType<SymbolType> &g = data[ind];
            tmp = hs_(data[ind]);
            ++ret;
            ret *= tmp;
            ind = (ind + 1) & mask;
        }
        return ret;
    }
    template<typename SymbolType, typename StringType>
    uint64_t operator()(const NGramType<SymbolType> &ngram, StringType &tmp) const {
        tmp.clear();
        for(const auto &gram: ngram)
            tmp.insert(tmp.back(), gram.begin(), gram.end());
        return hs_(tmp);
    }
};

using NGramHasher = NGramHasherBase<clhasher>;

union CPtr {
    char *nc_;
    const char *c_;
};

class ASCIITextSpacer {
    const char  *ptr_;
    mutable size_t index_;
    size_t l_;
    mutable std::pair<const char *, size_t> construct_;
public:
    ASCIITextSpacer(const char *d, size_t n): ptr_(d), index_(0), l_(n), construct_(nullptr, 0) {
        std::fprintf(stderr, "Made ASCIITextSpacer\n");
    }
    ASCIITextSpacer(const char *d): ASCIITextSpacer(d, std::strlen(d)) {}
    std::pair<const char *, size_t> *operator()() const {
        size_t nextind;
        while(index_ < l_ && !std::isalnum(ptr_[index_])) ++index_;
        if(index_ == l_) {
            return nullptr;
        }
        construct_.first = ptr_ + index_;
        while(index_ < l_ && !std::isspace(ptr_[index_])) ++index_;
        nextind = index_;
        while(std::ispunct(ptr_[index_ - 1])) --index_;
        construct_.second = ptr_ + index_ - construct_.first;
        index_ = nextind;
#if 0
        std::fprintf(stderr, "Returning construct with %p/%zu\n", (void *)construct_.first, construct_.second);
#endif
        return &construct_;
    }
};

template<typename SymbolType=char, typename HashStruct=clhasher>
class NGrammer {
#if 0
    union Handle {
        std::FILE *fp_;
        gzFile *gzfp_;
        Handle() {fp_ = nullptr;}
    };
    Handle h_; // Does not own either!
    CPtr  cp_; // Contains a char pointer of either const or nonconst
#endif
    uint32_t            l_;
    uint8_t mutable_seq_:1;
    uint16_t         n_:15;
    NGramType<SymbolType>    deque_;
    NGramHasherBase<HashStruct> hasher_;
public:
    template<typename... Args>
    NGrammer(unsigned n, bool mutable_seq, Args &&... args): mutable_seq_(mutable_seq), n_(n), deque_(n), hasher_(std::forward<Args>(args)...) {
        if(n > n_) throw std::runtime_error(ks::sprintf("NGrammer can only handle n of up to %zu", size_t(1 << 15)).data());
        std::fprintf(stderr, "Made NGrammer\n");
    }
#if 0
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
#endif
    template<typename... Args>
    NGramType<SymbolType> *next(Args &&... args) {
        switch(n_ - deque_.size()) {
            case 0:
                deque_.push_pop(std::forward<Args>(args)...);
                return &deque_;
            case 1:
                deque_.emplace_back(std::forward<Args>(args)...);
                return &deque_;
            default:
                deque_.emplace_back(std::forward<Args>(args)...);
                return nullptr;
        }
        __builtin_unreachable();
    }
    template<typename Functor, typename StringFunctor>
    void for_each(const Functor &func, const StringFunctor &sfunc) {
        NGramType<SymbolType> *g;
        using SFuncRetType = decltype(sfunc());
        SFuncRetType tmp;
        static_assert(std::is_pointer_v<SFuncRetType>, "StringFunctor must return a pointer.");
        {
            int i = 0;
            while((tmp = sfunc()) == nullptr && i++ < 20); // Maximum number of empty characters in a row.
            LOG_DEBUG("tmp: %p. i: %i\n", (void *)tmp, i);
            if(tmp == nullptr) {
                return;
            }
        }
        do {
            if constexpr(std::is_same_v<SFuncRetType, std::pair<const char *, size_t>*>) {
                if((g = this->next(tmp->first, tmp->second))) func(*g);
            } else {
                if((g = this->next(*tmp))) func(*g);
            }
        } while((tmp = sfunc()));
    }
    template<typename Functor, typename StringFunctor>
    void for_each_hash(const Functor &func, const StringFunctor &sfunc) {
        this->for_each([&](const NGramType<SymbolType> &v) {
            func(hasher_(v));
        }, sfunc);
    }
};

}
