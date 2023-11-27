#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "lossy_compression.h"

using namespace std;

pthread_mutex_t mutex;
const int NUM_THREADS = 8;
typedef struct CompressInfo{
public:
    vector<string> fileList;
    string outputPath;
    int instruct;
} CompressInfo;

// The thread function for compressing images
void* compressImg(void* arg) {
    CompressInfo* info = (CompressInfo*)arg;
    vector<string> fileList = info->fileList;
    string outputPath = info->outputPath;
    int instruct = info->instruct;
    for(string f : fileList){
        // TO DO: Perform the image compression using the file Name...
        vector<vector<vector<uint8_t>>> imgData;
        read_uncompressed(f, imgData);
        ofstream outfile;
        outfile.open("f.bin", ios::app | ios::binary);
    }

    return NULL;
}

int main(int argc, char* argv[]){
    // argv Parameters: [1]: Img folder path, [2]: Output folder path, [3]: Compression instruction
    // To test, we use BMP image files, and compress it.
    string folderPath = argv[1];
    string outputPath = argv[2];
    string instruction = argv[3];

    unsigned int fileCount = 0;
    DIR *p_dir;
    vector<string> fileList;
    folderPath = folderPath.append("/");
    p_dir = opendir(folderPath.c_str());
    if( p_dir == NULL)  cerr<< "can't open :" << folderPath << endl;

    struct dirent *p_dirent;
    while ( p_dirent = readdir(p_dir))
    {   
        string tmpFileName = p_dirent->d_name;
        if( tmpFileName == "." || tmpFileName == "..")
        {
            continue;
        }
        string extension = tmpFileName.substr(tmpFileName.length() - 4);
        if( extension == ".bmp")
        {
            fileList.push_back(folderPath+tmpFileName);
            fileCount++;
        }
    }
    closedir(p_dir);

    // assign threads to compress the images.
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[NUM_THREADS];
    CompressInfo info[NUM_THREADS];
    unsigned int imgPerThread = fileCount / NUM_THREADS, j = 0;;
    for (int i = 0; i < NUM_THREADS-1; i++) {
        info[i].fileList = vector<string>(fileList.begin() + j, fileList.begin() + j + imgPerThread);
        info[i].outputPath = outputPath;
        info[i].instruct = stoi(instruction);
        j += imgPerThread;
    }
    info[NUM_THREADS-1].fileList = vector<string>(fileList.begin() + j, fileList.end());
    info[NUM_THREADS-1].outputPath = outputPath;
    info[NUM_THREADS-1].instruct = stoi(instruction);

    for(int i = 0; i < NUM_THREADS; i++){
        pthread_create(&threads[i], NULL, compressImg, &info[i]);
    }

    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}
