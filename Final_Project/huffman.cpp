#include "huffman.h"

void ReadBMP(const string &filename, vector<uint8_t>& buffer, unsigned int (&freq)[256]){
    // Read the file in
    ifstream file(filename);
    buffer.clear();
    if (!file) {
        cout << "ERROR: Couldn't open input file " << filename << endl;
        return;
    }
    file.seekg(0,std::ios::end);
    streampos length = file.tellg();
    file.seekg(0,std::ios::beg);
    buffer.resize(length);
    file.read((char*) (&buffer[0]),length);
    file.close();
    
    // make sure this is a BMP file (first 2 chars are 'B' and 'M')
    if ((char) buffer[0] != 'B' || (char) buffer[1] != 'M'){
        cout << "ERROR: Input file is not a BMP" << endl;
        return;
    }
    
    for(uint8_t c : buffer){
        freq[(int)c]++;
    }
}

// Build the Huffman Tree
HuffmanNode* BuildHuffmanTree(unsigned int (&freq)[256]){
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompFreq> minHeap;

    // Create a leaf node for each symbol and add it to the priority queue
    for (uint8_t i=0; i != 255; i++) {
        if (freq[i] > 0) {
            HuffmanNode* node = new HuffmanNode(freq[i], i);
            minHeap.push(node);
        }
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

void BuildHeader(HuffmanNode *rt, vector<bool> &header)
{
    if(rt == NULL) return;
    if(rt->left == NULL && rt->right == NULL){
        header.push_back(true);
        header.push_back(rt->symbol);
    }
    header.push_back(false);
    BuildHeader(rt->left, header);
    BuildHeader(rt->right, header);
}

void encode(unordered_map<uint8_t, string> & table, HuffmanNode* rt, const vector<uint8_t>& buffer, const string &outputfile){
    string outputStr;
    vector<bool> outputBits;
    BuildHeader(rt, outputBits);
    unsigned int headerSize = outputBits.size();
    cout << "Header size: " << headerSize << endl;

    for(uint8_t c : buffer){
        for(char bit :  table[c]){
            outputBits.push_back(bit == '1');
        }
    }
    ofstream outFile(outputfile, ios::binary);
    //First write the header size into the file using 2 bytes
    bitset<16> headerSizeBits(headerSize);
    outFile.write((char*)&headerSizeBits, sizeof(headerSizeBits)/4);
    cout << headerSizeBits << endl;
    WriteBitsToFile(outputBits, outFile);
    // Close the file
    outFile.close();
}

void WriteBitsToFile(const vector<bool>& bits, ofstream & outFile) {
    // Open a file for writing in binary mode
    cout << "Writing bits to file..." << endl;
    // Convert bools vector to bytes and write to the file
    for (size_t i = 0; i < bits.size(); i += 8) {
        char byte = 0;
        for (size_t j = 0; j < 8 && i + j < bits.size(); ++j) {
            byte |= (bits[i + j] << (7 - j));
        }
        outFile.put(byte);
    }
}

HuffmanNode* Compress(const string &filename, const string &outputfile){
    vector<uint8_t> buffer; 
    unsigned int freq[256] = {};
    ReadBMP(filename, buffer, freq);

    HuffmanNode* rt = BuildHuffmanTree(freq);
    unordered_map<uint8_t, string> table;
    BuildMapping(rt, table, "");
 
    encode(table, rt, buffer, outputfile);
    return rt;
}

void Decompress(const string &filename, const string &outputfile){
    // Read the file in
    ifstream file(filename, ios::binary);
    if (!file) {
        cout << "ERROR: Couldn't open input file " << filename << endl;
        return;
    }
    file.seekg(0,std::ios::end);
    streampos length = file.tellg();
    file.seekg(0,std::ios::beg);
    vector<uint8_t> buffer(length);
    file.read((char*) (&buffer[0]),length);
    file.close();

    //The first 2 bytes are the header size
    unsigned int headerSize = (buffer[1] << 8) | buffer[0];
}