#ifndef HUFFMAN_H_
#define HUFFMAN_H_
#include <cstdint>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <stdint.h>

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

void ParseData(vector<vector<vector<uint8_t>>>& data, unordered_map<int, int>& freq);

HuffmanNode* BuildHuffmanTree(unordered_map<uint8_t, int> freq);

void BuildMapping(HuffmanNode* root, unordered_map<uint8_t, string>& table, string code);
#endif /* HUFFMAN_H_ */