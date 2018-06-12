# dnlp
Daniel's NLP library

This is a relatively new project. Currently, all that's supported are utilities for iterating over ngrams in text corpuses, and, as of yet, only ASCII is supported.
The goal is to reuse this code in a variety of projects.

## parse.h
Parses a text file or a string, iterating efficiently over ngrams by using a circular buffer and short-string-optimized strings.

## testparse.cpp
Simply puts all ngrams from a file into a hyperloglog and reports its cardinality. This has obvious applications in indexing and approximate counting for natural language processing applications.
