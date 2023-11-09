#include "encode.h"
#include <iostream>
using namespace std;

uint64_t ColumnHash(string const& word) {
    // According to a test on the column.txt, 
    // one feature for all words are that they are less than 12 characters.
    // So, if we use 5 bits to represent each character, and each 5 bit represents a letter in the hash value,
    // a 64-bit hash value can hold 12 letters, which is enough for each word in the file to garantee a UNIQUE hash value!
    // One other benefit is that in fact we can decode this value and turn it back into a word, very useful
    const int mul = 32;
    int64_t hash_value = 0;
    for (char c : word) {
        hash_value += hash_value * mul + (c-'a'+1);
    }
    return hash_value;
}
uint64_t Hash(string const& word) {
    const int mul = 32;
    int64_t hash_value = 0;
    for (char c : word) {
        hash_value += hash_value * mul + (c-'a'+1);
    }
    return hash_value;
}

uint64_t encode(string word, long index)
{
    int64_t hash_value = ColumnHash(word);
    //cout << hash_value << endl;
    return hash_value;
}

void decode(string encodedWord)
{
}
