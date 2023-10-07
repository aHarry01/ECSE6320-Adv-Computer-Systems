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
    int** cols;
    int rIndex, rLenth, cLength;
    int** FINAL_RESULT;
    int i;
} PartialMatrix;

// The thread function
void* rc_Multiplication(void* arg) {
    PartialMatrix* p = (PartialMatrix*)arg;
    //TO DO: adjust the parameters to the correct versions based on PartialMatrix
    int r = p->rIndex, c = p->cIndex, len = p->rcLenth;
    int* rowArr = p->rowArr, *colArr = p->colArr;
    int** RESULT_MATRIX = p->FINAL_RESULT;
    int count = p->i;

    // Calculate the result of row*colume by element
    int res = 0;
    for(int i = 0; i < len; i++){
        res += rowArr[i] * colArr[i];
    }

    if(RESULT_MATRIX[r][c] != 0){
        throw runtime_error("The place to write the result has value..." + to_string(r) + " " + to_string(c));
    }

    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&mutex);
    // Access shared data
    RESULT_MATRIX[r][c] = res;
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
}

int main(int argc, char* argv[]) {
    // argv Parameters: [1]: number of threads; [2]: Matrix1; [3]: Matrix2; 
    // [4]: Matrix1_r; [5]: Matrix1_c; [6]: Matrix2_r; [7]: Matrix2_c;
    if (argc != 8){
        cout << "Usage: ./rc_Multiplication [number of threads] [Matrix1] [Matrix2]" << endl;
        return 0;
    }
    const int NUM_THREADS = stoi(argv[1]);
    int r1 = stoi(argv[4]), c1 = stoi(argv[5]);
    int r2 = stoi(argv[6]), c2 = stoi(argv[7]);
    int** M1 = readMatrix(argv[2], r1, c1);
    int** M2 = transposeMatrix(readMatrix(argv[3], r2, c2), r2, c2);

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // The two matrix to be multiplied and the place to store the answer
    int tmp1[] = {M2[0][0], M2[1][0]};
    int tmp2[] = {M2[0][1], M2[1][1]};
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
    
    PartialMatrix p[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    // Create threads
    for(int i = 0; i < NUM_THREADS; i++){
        //TO DO: redesign the method to asign the partial matrix to threads
        //To Do: Correct all the partial matrices correctly.
        pthread_create(&threads[i], NULL, rc_Multiplication, &p[i]);
    }
    cout << "Threads are created..." << endl;

    // Wait for threads to finish
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    //Print the result
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
            cout << res[i][j] << " ";
        }
        cout << endl;
    }    
    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    //TO DO: free memory
    return 0;
}