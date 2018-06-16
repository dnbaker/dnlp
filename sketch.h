#pragma once
#include <cinttypes>
#include "parse.h"
#include "hll/hll.h"

namespace dnlp {
using namespace sketch;
using namespace std::literals;

namespace {
enum CompReading: unsigned {
    UNCOMPRESSED,
    GZ,
    AUTODETECT
};
}

void fill_from_file(ks::string &ks, std::FILE *fp, size_t size=0) {
    int c;
    while((c = std::fgetc(fp)) >= 0) ks.putc_(c);
    ks.terminate();
}
void fill_from_file(ks::string &ks, gzFile fp) {
    int c;
    while((c = gzgetc(fp)) >= 0) ks.putc_(c);
    ks.terminate();
}

ks::string load_from_gz(const char *fn) {
    ks::string ks;
    gzFile fp = gzopen(fn, "rb");
    if(fp == nullptr) throw std::runtime_error("Could not open file at "s + fn);
    //const size_t sz = filesize(fn);
    fill_from_file(ks, fp);
    gzclose(fp);
    return ks;
}
ks::string load_from_file(const char *fn) {
    ks::string ks;
    std::FILE *fp = std::fopen(fn, "rb");
    if(fp == nullptr) throw std::runtime_error("Could not open file at "s + fn);
    //const size_t sz = filesize(fn);
    fill_from_file(ks, fp);
    std::fclose(fp);
    return ks;
}
#if 0
    clhasher hasher(137, 167);
#endif
template<typename T, typename Hasher, typename... Args>
T make_sketch(const char *fn, unsigned mode, unsigned n, const Hasher &hasher, Args &&... args) {
    T ret(std::forward<Args>(args)...);
    ks::string ks;
    switch(mode) {
        case UNCOMPRESSED: {
            ks = load_from_file(fn);
            break;
        }
        case GZ: {
            ks = load_from_gz(fn);
            break;
        }
        case AUTODETECT: {
            ks = (std::strcmp(fn + std::strlen(fn) - 3, ".gz") == 0) ? load_from_gz(fn): load_from_file(fn);
            break;
            // Could expand to read for the magic number...
        }
    }
    ASCIITextSpacer spacer(ks.data(), ks.size());
    NGrammer<> ng(n);
    ng.for_each_hash([&](uint64_t val){
        ret.addh(val);
    }, spacer);
    return ret;
}

class NGramHLLGlobber {
    // N - ngram
    // Array of HLLs, one for each N
    // Read from file, hashing each subngram into each of these
};
class ContextHLLGlobber {
    // N - ngram
    // Array of HLLs, one for each N
    // Read from file, hashing each subngram into each of these
};

class NGramDistrep {
    // Use NGramHLLGlobber and/or ContextHLLGlobber, except as a k/v store from ngram to its contexts.
};

} // namespace dnlp
