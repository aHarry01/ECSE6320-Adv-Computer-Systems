#include "encode.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <set>
#include <immintrin.h>

#include "pthread.h"
#include "zlib.h"

using namespace std;

const unsigned int WORDS_PER_THREAD = 20000000;
const string FILE_PATH = "Column.txt";
const string ENCODED_FILE_PATH = "EncodedColumn.bin";
const string COMPRESSED_FILE_PATH = "CompressedColumn.bin";
const string WORD_TO_LOOKUP = "thsgtyw";
const string PREFIX_TO_LOOKUP = "pik";
pthread_mutex_t mutex;  // A mutex to protect shared data

unordered_map<string, uint64_t> columnMap;

typedef struct EncodeInfo{
public:
    string* words;
    unsigned int startIndex, wordNum;
} EncodeInfo;

void* EncodeWords(void* arg){
    //Multi thread function to encode the words and save them into a file
    EncodeInfo* info = (EncodeInfo*)arg;
    string* words = info->words;
    unsigned int startIndex = info->startIndex, len = info->wordNum;
    vector<uint64_t> encodedValues;

    for(unsigned int i = 0; i < len; i++){
        uint64_t hashValue = ColumnHash(words[i]);
        encodedValues.push_back(hashValue);
    }

    ofstream outfile;
    outfile.open(ENCODED_FILE_PATH, ios::app | ios::binary);

    pthread_mutex_lock(&mutex);
    for(unsigned int i = 0; i < len; i++){
        columnMap[words[i]] = encodedValues[i];
    }
    outfile.write(reinterpret_cast<const char*>(encodedValues.data()), encodedValues.size() * sizeof(uint64_t));
    outfile.close();

    pthread_mutex_unlock(&mutex);
    delete[] words;
    delete info;
    return NULL;
}

unsigned int EncodeFromFile(string filePath){
    ifstream infile(filePath);
    if(!infile.is_open()){
        throw runtime_error("Unable to open file");
    }

    string* words = new string[WORDS_PER_THREAD];
    string word;
    vector<pthread_t*> threads;
    unsigned int i = 0, wordCount = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while(infile >> word){ 
        wordCount++;
        words[i] = word;
        i++;

        if (i == WORDS_PER_THREAD){
            EncodeInfo* info = new EncodeInfo();
            info->words = words;
            info->startIndex = wordCount-i;
            info->wordNum = i;

            //Create a thread to encode the words
            pthread_t* thread = new pthread_t;
            pthread_create(thread, NULL, EncodeWords, info);
            threads.push_back(thread);

            //Refresh counters for new lines
            words = new string[WORDS_PER_THREAD];
            i = 0;
        }
    }
    // Encode the remaining words
    EncodeInfo* info = new EncodeInfo();
    info->words = words;
    info->startIndex = wordCount-i;
    info->wordNum = i;
    pthread_t* thread = new pthread_t;
    pthread_create(thread, NULL, EncodeWords, info);
    threads.push_back(thread);

    cout << "Words in file: " << wordCount << endl;

    for (unsigned int x = 0; x < threads.size(); x++){
        pthread_join(*threads[x], NULL);
    }
    
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "Encode finished in " << elapsed.count() << " seconds, Compressing..." << endl;
    infile.close();
    if (Compress(ENCODED_FILE_PATH, COMPRESSED_FILE_PATH) == Z_OK){
        remove(ENCODED_FILE_PATH.c_str());
        cout << "Compression finished" << endl;
        return wordCount;
    }
    throw runtime_error("Compression failed");
}

// ----- Single word search ------------

//Search without using dictionary encoding for comparison purposes
bool word_query_no_encoding(const string &filePath, const string &query, vector<int> &indices){
    ifstream infile(filePath);
    string word;
    vector<string> words;
    int i = 0;
    while(infile >> word){ 
        words.push_back(word);
    }

    // don't include file loading in performance measuring
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < words.size(); i++){
        if (words[i] == query){
            indices.push_back(i);
        }
        i++;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    
    bool ret = true;
    if (indices.size() == 0){
        cout << "Word not found" << endl;
    } else {
        cout << "Word found in " << indices.size() << " locations" << endl; 
    }
   
    cout << "Word lookup with no encoding took " << elapsed.count() << " seconds" << endl;
    return ret;
}

// Check if a word exists and return indices of all occurences if it does
bool word_query(const string &query, vector<uint64_t> &values, vector<int> &indices, bool use_simd){
    bool ret = false;
    auto start = std::chrono::high_resolution_clock::now();
    if (columnMap.find(query) == columnMap.end()){
        cout << "Word not found" << endl;
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        cout << "Word lookup with encoding took " << elapsed.count() << " seconds" << endl;
    } 
    // Lookup locations using SIMD instructions
    else if (use_simd) {
        uint64_t code = columnMap[query];
        // use SIMD instructions to compare 4 codes at a time
        __m256i code_compare = _mm256_set_epi64x(code, code, code, code);
        __m256i mask = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
        __m256i data;
        uint64_t results[4] = {0, 0, 0, 0};
        for (int i = 0; i < values.size()-8; i+=4){
            data = _mm256_loadu_si256((__m256i*) &values[i]);
            data = _mm256_cmpeq_epi64(data, code_compare);
            _mm256_maskstore_epi64((long long int*) results, mask, data);
            //calculate indices from comparison results
            for (int x = i; x < i+4; x++){
                if (results[x-i] != 0)
                    indices.push_back(x);
            }
        }
        cout << "Word found in " << indices.size() << " locations" << endl; 
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        cout << "Word lookup with encoding and SIMD took " << elapsed.count() << " seconds" << endl;
    } 
    // Lookup locations without SIMD instructions
    else {
        uint64_t code = columnMap[query];
        for(int i = 0; i < values.size(); i++){
            if (values[i] == code){
                indices.push_back(i);
            }
        }
        cout << "Word found in " << indices.size() << " locations" << endl; 
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        cout << "Word lookup with encoding took " << elapsed.count() << " seconds" << endl;
    }

    return ret;
}

// ----- Prefix search ------------

// Get the locations of all words that begin with this prefix with no dictionary encoding
void prefix_query_no_encoding(const string &filePath, const string &prefix, vector<int> &indices){

    ifstream infile(filePath);
    string word;
    std::set<string> unique_words;
    auto start = std::chrono::high_resolution_clock::now();
    int i = 0;
    while(infile >> word){ 
        if (word.compare(0, prefix.size(), prefix) == 0){
            indices.push_back(i);
            if (unique_words.find(word) == unique_words.end()){
                unique_words.insert(word);
            }
        }
        i++; 
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    if (indices.size() == 0){
        cout << "Prefix not found" << endl;
    } else {
        cout << "Prefix found in " << unique_words.size() << " unique words and " << indices.size() << " total locations" << endl; 
    }
   
    cout << "Prefix lookup with no encoding took " << elapsed.count() << " seconds" << endl;
}

// Get the locations of all words that begin with this prefix with dictionary encoding
void prefix_query(const string &prefix, vector<uint64_t> &values, vector<int> &indices, bool use_simd){

    // use dictionary to get all the words that start with the prefix
    vector<string> unique_words;
    unordered_map<string, uint64_t>::iterator itr;
    auto start = std::chrono::high_resolution_clock::now();

    for (itr = columnMap.begin(); itr != columnMap.end(); itr++){
        // check if this word has the prefix
        if ((itr->first).compare(0, prefix.size(), prefix) == 0){
            unique_words.push_back(itr->first);
            uint64_t code = itr->second;

            // now search for the locations for each word
            if (use_simd) {
                // use SIMD instructions to compare 4 codes at a time
                __m256i mask = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
                __m256i data;
                uint64_t results[4] = {0, 0, 0, 0};
                for (int i = 0; i < values.size()-8; i+=4){
                        __m256i code_compare = _mm256_set_epi64x(code, code, code, code);
                        data = _mm256_loadu_si256((__m256i*) &values[i]);
                        data = _mm256_cmpeq_epi64(data, code_compare);
                        _mm256_maskstore_epi64((long long int*) results, mask, data);
                        //calculate indices from comparison results
                        for (int x = i; x < i+4; x++){
                            if (results[x-i] != 0)
                                indices.push_back(x);
                        }
                }
            } else {
                for(int i = 0; i < values.size(); i++){
                    if (values[i] == code){
                        indices.push_back(i);
                    }
                }
            }
        }
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    if (indices.size() == 0){
        cout << "Prefix not found" << endl;
    } else {
        cout << "Prefix found in " << unique_words.size() << " unique words and " << indices.size() << " total locations" << endl; 
    }
    cout << "Prefix lookup with encoding took " << elapsed.count() << " seconds" << endl;
}

int main(int argc, char* argv[]){
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    unsigned int num = EncodeFromFile(FILE_PATH);
    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    vector<uint64_t> values = Decompress(COMPRESSED_FILE_PATH, num);
    cout << values.size() << endl;
    // for (int i = 0; i < values.size(); i++){
    //     cout << values[i] << endl;
    // }

    // ---- Queries
    // First measure the lookup speed with no encoding
    vector<int> indices;
    word_query_no_encoding(FILE_PATH, WORD_TO_LOOKUP, indices);
    vector<int> indices2;
    word_query(WORD_TO_LOOKUP, values, indices2, false);
    vector<int> indices3;
    word_query(WORD_TO_LOOKUP, values, indices3, true);

    indices.clear();
    indices2.clear();
    indices3.clear();
    prefix_query_no_encoding(FILE_PATH, PREFIX_TO_LOOKUP, indices);
    prefix_query(PREFIX_TO_LOOKUP, values, indices2, false);
    prefix_query(PREFIX_TO_LOOKUP, values, indices3, true);

    // 
}