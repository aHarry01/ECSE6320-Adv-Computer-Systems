#include "encode.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>

#include "pthread.h"
#include "zlib.h"

using namespace std;

const unsigned int WORDS_PER_THREAD = 20000000;
const string FILE_PATH = "Column.txt";
const string ENCODED_FILE_PATH = "EncodedColumn.bin";
const string COMPRESSED_FILE_PATH = "CompressedColumn.bin";
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
    infile.close();
    cout << "Encode finished, Compressing..." << endl;

    if (Compress(ENCODED_FILE_PATH, COMPRESSED_FILE_PATH) == Z_OK){
        remove(ENCODED_FILE_PATH.c_str());
        cout << "Compression finished" << endl;
        return wordCount;
    }
    throw runtime_error("Compression failed");
}

int main(int argc, char* argv[]){
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    unsigned int num = EncodeFromFile(FILE_PATH);
    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    vector<uint64_t> values = Decompress(COMPRESSED_FILE_PATH, num);
    cout << values.size() << endl;
}