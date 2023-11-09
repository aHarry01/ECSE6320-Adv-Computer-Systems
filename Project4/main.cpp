#include "encode.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <vector>

#include <pthread.h>
#include <immintrin.h>

using namespace std;

const unsigned int WORDS_PER_THREAD = 10000000;
const string FILE_PATH = "Column.txt";
const string ENCODED_FILE_PATH = "EncodedColumn.txt";
pthread_mutex_t mutex;  // A mutex to protect shared data

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
    uint64_t *encodedValues = new uint64_t[WORDS_PER_THREAD];

    cout << "Thread start, index start from " << startIndex << endl;

    for(unsigned int i = 0; i < len; i++){
        encodedValues[i] = ColumnHash(words[i]);
    }

    // TODO: Further compress the value, and save them into a file

    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&mutex);
    // Access shared data
    ofstream outfile;
    outfile.open(ENCODED_FILE_PATH, ios::app);
    for(unsigned int i = 0; i < len; i++){
        outfile << encodedValues[i] << " ";
    }
    outfile.close();
    cout << "Thread finish, index start from " << startIndex << ", Encoded " << len << " words" << endl;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex); 

    delete[] words;
    delete info;
    return NULL;
}

void EncodeFromFile(string filePath){
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
}

int main(int argc, char* argv[]){
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    
    EncodeFromFile(FILE_PATH);

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);
}