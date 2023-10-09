#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// A mutex to protect shared data
pthread_mutex_t mutex;



typedef struct PartialMatrix{
// Only one col and one row
public:
    int** rows;
    int** M2T;
    int rIndex, rLenth, cLength;
    int** FINAL_RESULT;
} PartialMatrix;

// The thread function
void* rc_Multiplication(void* arg) {
    PartialMatrix* p = (PartialMatrix*)arg;
    int rIndex = p->rIndex, rLenth = p->rLenth, cLength = p->cLength;
    int** rows = p->rows, **M2T = p->M2T, **FINAL_RESULT = p->FINAL_RESULT;

    int partialRes[rLenth][cLength];
    for(int i = 0; i < rLenth; i++){
        for(int j = 0; j < cLength; j++){
            int tmp = 0;
            for(int k = 0; k < cLength; k++){
                tmp += rows[i][k] * M2T[j][k];
            }
            partialRes[i][j] = tmp;
        }
    }

    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&mutex);
    // Access shared data
    for(int i = 0, r = rIndex; i < rLenth; i++, r++){
        for(int j = 0; j < cLength; j++){
            FINAL_RESULT[r][j] = partialRes[i][j];
        }
    }
    // Unlock the mutex
    pthread_mutex_unlock(&mutex); 
    return NULL;
}

int** readMatrix(string fileName, int r, int c){
    ifstream inf;
    string line;
    int i=0, j, n;
    int** res = new int*[r];
    for(int i = 0; i < n; i++){
        res[i] = new int[c];
    }
    inf.open(fileName);
    while(getline(inf, line)){
        stringstream iss(line);
        j = 0;
        while (iss >> n)
        {
            res[i][j] = n;
            j++;
        }
        i++;
    }
    inf.close();
    return res;
}

int** transposeMatrix(int** M, int r, int c){
    //Transpose a matrix
    int** T = new int*[c];
    for(int i = 0; i < c; i++){
        T[i] = new int[r];
        for(int j = 0; j < r; j++){
            T[i][j] = M[j][i];
        }
    }
    //Free M
    for(int i = 0; i < r; i++){
        delete[] M[i];
    }
    return T;
}

int main(int argc, char* argv[]) {
    // argv Parameters: [1]: number of threads; [2]: Matrix1; [3]: Matrix2; 
    // [4]: Matrix1_r; [5]: Matrix1_c; [6]: Matrix2_r; [7]: Matrix2_c;
    if (argc != 8 || stoi(argv[5])!= stoi(argv[6])){
        //Output an error about number of arguments or can't multiply matrices
        return 0;
    }
    const int NUM_THREADS = stoi(argv[1]);
    int r1 = stoi(argv[4]), c1 = stoi(argv[5]);
    int r2 = stoi(argv[6]), c2 = stoi(argv[7]);
    int** M1 = readMatrix(argv[2], r1, c1);
    cout << "Matrix1 read..." << endl;
    int** M2T = transposeMatrix(readMatrix(argv[3], r2, c2), r2, c2);   //Transpose of M2
    cout << "Matrix2 read and processed" << endl;

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // Initialize the result matrix
    int** res;
    res = new int*[r1];
    for(int i = 0; i < r1; i++){
        res[i] = new int[c2];
    }
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
            res[i][j] = 0;
        }
    }

    cout << "Result matrix initialized..." << endl;

    //Initialize all partial matrices
    int rowsRerThread = r1/NUM_THREADS; // The task was to multiply big matrices, so we assume that r1 >> NUM_THREADS
    pthread_t threads[NUM_THREADS];
    PartialMatrix p[NUM_THREADS];
    for (int i = 0, r = 0; i < NUM_THREADS; i++, r += rowsRerThread) {
        p[i].rows = &M1[r];
        p[i].M2T = M2T;
        p[i].rIndex = r;
        p[i].rLenth = i == NUM_THREADS-1 ? r1-r : rowsRerThread;
        p[i].cLength = c2;
        p[i].FINAL_RESULT = res;
    }

    cout << "Partial mateix created..." << endl;

    // Create threads
    for(int i = 0; i < NUM_THREADS; i++){
        cout << "Thread " << i << " is creating..." << endl;
        pthread_create(&threads[i], NULL, rc_Multiplication, &p[i]);
    }
    cout << "Threads are created..." << endl;

    // Wait for threads to finish
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    //Print the result
    for(int i = 0; i < 11; i++){
        for(int j = 0; j < 11; j++){
            cout << res[i][j] << " ";
        }
        cout << endl;
    }    
    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    //TO DO: free memory
    return 0;
}