#include "huffman.h"

void ReadBMP(const string &filename, vector<uint8_t>& buffer, unsigned int (&freq)[256]){
    // Read the file in
    ifstream file(filename);
    buffer.clear();
    if (!file) {
        cout << "ERROR: Couldn't open input file " << filename << endl;
        return;
    }
    file.seekg(0,ios::end);
    streampos length = file.tellg();
    file.seekg(0,ios::beg);
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

void BuildMappingSIMD(HuffmanNode* root, unordered_map<uint8_t, string>& table, string code) {
    if (root == nullptr) return;

    // SIMD vectors to store symbols and corresponding codes
    SimdCode simdCode;

    // Initialize SIMD vectors with zeros
    simdCode.symbols = _mm_setzero_si128();
    simdCode.codes = _mm_setzero_si128();

    // Process left child
    if (root->left != nullptr) {
        BuildMappingSIMD(root->left, table, code + "0");

        // Convert the code to an SIMD vector of 8-bit integers
        __m128i codeVector = _mm_set1_epi8(stoi(code + "0", nullptr, 2));

        // Update SIMD vectors
        simdCode.symbols = _mm_insert_epi8(simdCode.symbols, root->left->symbol, 0);
        simdCode.codes = _mm_insert_epi8(simdCode.codes, _mm_extract_epi8(codeVector, 0), 0);

    }

    if (root->right != nullptr) {
        BuildMappingSIMD(root->right, table, code + "1");

        // Convert the code to an SIMD vector of 8-bit integers
        __m128i codeVector = _mm_set1_epi8(stoi(code + "1", nullptr, 2));
        simdCode.symbols = _mm_insert_epi8(simdCode.symbols, root->right->symbol, 0);
        simdCode.codes = _mm_insert_epi8(simdCode.codes, _mm_extract_epi8(codeVector, 0), 0);

    }

    // Extract values from SIMD vectors and update the table
    alignas(16) array<uint8_t, 16> symbolsArray;
    alignas(16) array<uint8_t, 16> codesArray;

    _mm_store_si128(reinterpret_cast<__m128i*>(&symbolsArray[0]), simdCode.symbols);
    _mm_store_si128(reinterpret_cast<__m128i*>(&codesArray[0]), simdCode.codes);

    for (int i = 0; i < 16; ++i) {
        table[symbolsArray[i]] = bitset<8>(codesArray[i]).to_string();
    }
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

void encode(unordered_map<uint8_t, string> & table, HuffmanNode* rt, const vector<uint8_t>& buffer, const string &outputfile, bool useSIMD){
    string outputStr;
    vector<bool> outputBits;
    BuildHeader(rt, outputBits);
    unsigned int headerSize = outputBits.size();

    for(uint8_t c : buffer){
        for(char bit :  table[c]){
            outputBits.push_back(bit == '1');
        }
    }
    ofstream outFile(outputfile, ios::binary);
    //First write the header size into the file using 2 bytes
    bitset<16> headerSizeBits(headerSize);
    outFile.write((char*)&headerSizeBits, sizeof(headerSizeBits)/4);
    if(useSIMD){
        WriteBitsToFileSIMD(outputBits, outFile);
    }
    else{
        WriteBitsToFile(outputBits, outFile);
    }
    // Close the file
    outFile.close();
}

void WriteBitsToFile(const vector<bool>& bits, ofstream & outFile) {
    // Open a file for writing in binary mode
    // cout << "Writing bits to file..." << endl;
    // Convert bools vector to bytes and write to the file
    for (size_t i = 0; i < bits.size(); i += 8) {
        char byte = 0;
        for (size_t j = 0; j < 8 && i + j < bits.size(); ++j) {
            byte |= (bits[i + j] << (7 - j));
        }
        outFile.put(byte);
    }
}

void WriteBitsToFileSIMD(const vector<bool>& bits, ostream& outFile) {
    __m128i packedData = _mm_setzero_si128();
    int bitsProcessed = 0;

    for (bool bit : bits) {
        // Set the current bit in the packedData
        packedData = _mm_or_si128(_mm_slli_epi64(packedData, 1), _mm_set1_epi64x(bit));

        // Increment the count of processed bits
        ++bitsProcessed;

        // If 64 bits have been processed, write to file
        if (bitsProcessed == 64) {
            alignas(16) std::array<uint64_t, 2> packedArray;
            _mm_store_si128(reinterpret_cast<__m128i*>(&packedArray[0]), packedData);
            outFile.write(reinterpret_cast<const char*>(&packedArray[0]), sizeof(packedArray));
            packedData = _mm_setzero_si128();
            bitsProcessed = 0;
        }
    }

    // Write any remaining bits to file
    if (bitsProcessed > 0) {
        alignas(16) std::array<uint64_t, 2> packedArray;
        _mm_store_si128(reinterpret_cast<__m128i*>(&packedArray[0]), packedData);
        outFile.write(reinterpret_cast<const char*>(&packedArray[0]), sizeof(packedArray));
    }
}

HuffmanNode* Compress(const string &filename, const string &outputfile, bool useSIMD){
    vector<uint8_t> buffer; 
    unsigned int freq[256] = {};
    ReadBMP(filename, buffer, freq);

    HuffmanNode* rt = BuildHuffmanTree(freq);
    unordered_map<uint8_t, string> table;
    if(useSIMD)
        BuildMappingSIMD(rt, table, "");
    else
        BuildMapping(rt, table, "");
    // cout << "Header Size: " << table.size() << endl;
    // cout << "Original\t" << "Huffman Code\t" << "Frequency"<< endl;
    // for(auto kv : table){
    //     cout << hex << (int)kv.first << "\t\t" << kv.second << "\t\t" << dec << freq[(int)kv.first]<< endl;
    // }
 
    encode(table, rt, buffer, outputfile, useSIMD);
    return rt;
}

void Decompress(const string &filename, const string &outputfile){
    // Read the file in
    ifstream file(filename, ios::binary);
    if (!file) {
        cout << "ERROR: Couldn't open input file " << filename << endl;
        return;
    }
    file.seekg(0,ios::end);
    streampos length = file.tellg();
    file.seekg(0,ios::beg);
    vector<uint8_t> buffer(length);
    file.read((char*) (&buffer[0]),length);
    file.close();

    //The first 2 bytes are the header size
    unsigned int headerSize = (buffer[1] << 8) | buffer[0];

    //TODO: read the header, reconstruct huffmantree, read the rest of the bits.
}