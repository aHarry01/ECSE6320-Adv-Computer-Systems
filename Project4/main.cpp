#include "encode.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>

#include <pthread.h>
#include <zlib.h>

using namespace std;

const unsigned int WORDS_PER_THREAD = 20000000;
const string FILE_PATH = "Column.txt";
const string ENCODED_FILE_PATH = "EncodedColumn.bin";
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

    cout << "Thread start, index start from " << startIndex << endl;

    for(unsigned int i = 0; i < len; i++){
        uint64_t hashValue = ColumnHash(words[i]);
        encodedValues.push_back(hashValue);
        columnMap.insert(make_pair(words[i], hashValue));
    }

    ofstream outfile;
    outfile.open(ENCODED_FILE_PATH, ios::app | ios::binary);

    // pthread_mutex_lock(&mutex);
    // outfile.write(reinterpret_cast<const char*>(encodedValues.data()), encodedValues.size() * sizeof(uint64_t));
    // outfile.close();

    // Compress the data using zlib
    const void* data_ptr = encodedValues.data();
    const size_t data_size = encodedValues.size() * sizeof(uint64_t);

    const int compression_level = Z_BEST_COMPRESSION;
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    deflateInit(&stream, compression_level);

    stream.next_in = (Bytef*)data_ptr;
    stream.avail_in = data_size;

    std::vector<uint8_t> compressed_data;
    compressed_data.resize(deflateBound(&stream, data_size));
    stream.next_out = compressed_data.data();
    stream.avail_out = compressed_data.size();

    deflate(&stream, Z_FINISH);
    deflateEnd(&stream);

    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&mutex);

    outfile.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());
    outfile.close();

    // Unlock the mutex
    pthread_mutex_unlock(&mutex); 
    cout << "Thread finished" << endl;
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

    for (unsigned int x = 0; x < threads.size(); x++){
        pthread_join(*threads[x], NULL);
    }
    infile.close();
    cout << "Encode finished" << endl;
    return wordCount;
}

int main(int argc, char* argv[]){
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    
    unsigned int num = EncodeFromFile(FILE_PATH);


    // Test: read from the binary file generated
    ifstream infile(ENCODED_FILE_PATH, ios::binary);
    infile.seekg(0, std::ios::end);
    std::streampos file_size = infile.tellg();
    infile.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(file_size);
    infile.read(reinterpret_cast<char*>(data.data()), file_size);
    infile.close();

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    inflateInit(&stream);

    stream.next_in = data.data();
    stream.avail_in = data.size();

    std::vector<uint64_t> decompressed_data;
    decompressed_data.resize(num + 1024); //Should be enough...

    stream.next_out = reinterpret_cast<Bytef*>(decompressed_data.data());
    stream.avail_out = decompressed_data.size() * sizeof(uint64_t);

    inflate(&stream, Z_FINISH);
    inflateEnd(&stream);


    cout << decompressed_data[0] << endl;

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);
}