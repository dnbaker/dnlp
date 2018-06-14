#pragma once
#include "parse.h"
#include "hll/hll.h"

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
