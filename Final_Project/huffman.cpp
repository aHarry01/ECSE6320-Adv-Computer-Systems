#include "huffman.h"

void ParseData(vector<vector<vector<uint8_t>>>& data, unordered_map<int, int>& freq){
    unsigned int h = data.size(), w = data[0].size();
    vector<int> hex;
    for (vector<vector<uint8_t>> h : data){
        for (vector<uint8_t> pixel : h){
            int hexValue = (pixel[0]<<16) + (pixel[1]<<8) + pixel[2];
            hex.push_back(hexValue);
            freq[hexValue]++;
        }
    }
}

// Build the Huffman Tree
HuffmanNode* BuildHuffmanTree(unordered_map<uint8_t, int> freq){
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompFreq> minHeap;

    // Create a leaf node for each symbol and add it to the priority queue
    for (const auto& kv : freq) {
        HuffmanNode* node = new HuffmanNode(kv.second, kv.first);
        minHeap.push(node);
    }

    // Build the Huffman Tree
    while (minHeap.size() > 1) {
        HuffmanNode* left = minHeap.top();
        minHeap.pop();

        HuffmanNode* right = minHeap.top();
        minHeap.pop();

        HuffmanNode* newNode = new HuffmanNode(left->freq + right->freq, '\0');
        newNode->left = left;
        newNode->right = right;

        minHeap.push(newNode);
    }

    // The remaining node is the root of the Huffman Tree
    return minHeap.top();
}

void BuildMapping(HuffmanNode* root, unordered_map<uint8_t, string>& table, string code){
    // basic thought: map uint_8 to string, then when compressing transfer string to bits.
    if(root == NULL) return;
    if(root->left == NULL && root->right == NULL){
        table[root->symbol] = code;
        return;
    }
    BuildMapping(root->left, table, code + "0");
    BuildMapping(root->right, table, code + "1");
}