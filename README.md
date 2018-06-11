# dnlp
Daniel's NLP library

This is a relatively new project. Currently, all that's supported are utilities for iterating over ngrams in text corpuses, and, as of yet, only ASCII is supported.
The goal is to reuse this code in a variety of projects.

## parse.h
Parses a text file or a string, iterating efficiently over ngrams by using a circular buffer and short-string-optimized strings.

