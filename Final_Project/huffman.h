#ifndef HUFFMAN_H_
#define HUFFMAN_H_
#include <cstdint>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <bitset>
#include <immintrin.h>
using namespace std;

struct HuffmanNode{
    int freq;
    uint8_t symbol;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(int f, char s) : freq(f), symbol(s), left(NULL), right(NULL) {}
};

struct CompFreq {
    bool operator()(HuffmanNode* left, HuffmanNode* right) {
        return left->freq > right->freq;
    }
};

// Structure to store SIMD vectors of symbols and corresponding codes
struct SimdCode {
    __m128i symbols;
    __m128i codes;
};

void ReadBMP(const string &filename, vector<uint8_t>& buffer, unsigned int (&freq)[256]);

HuffmanNode* BuildHuffmanTree(unsigned int (&freq)[256]);
void BuildMapping(HuffmanNode* root, unordered_map<uint8_t, string>& table, string code);
void BuildMappingSIMD(HuffmanNode* root, unordered_map<uint8_t, string>& table, string code);
void BuildHeader(HuffmanNode* rt, vector<bool>& header);

void WriteBitsToFile(const vector<bool>& bits, ofstream & outFile);
void WriteBitsToFileSIMD(const vector<bool>& bits, ostream& outFile);

HuffmanNode* Compress(const string &filename, const string &outputfile, bool useSIMD);

void Decompress(const string &filename, const string &outputfile);
#endif /* HUFFMAN_H_ */