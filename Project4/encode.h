#ifndef ENCODE_H_
#define ENCODE_H_
#include <string>
using namespace std;

uint64_t ColumnHash(string const& word);
uint64_t Hash(string const& word);
uint64_t encode(string word, long index);
void decode(string encodedWord);
#endif