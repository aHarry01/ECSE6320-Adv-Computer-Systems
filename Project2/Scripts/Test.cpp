#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>
#include <stdexcept>
#include <iostream>
using namespace std;

// A mutex to protect shared data
pthread_mutex_t mutex;
const int NUM_THREADS = 4;
int** res;

typedef struct PartialMatrix{
public:
    int* rowArr;
    int* colArr;
    int rIndex, cIndex, rcLenth;
    int** FINAL_RESULT;
    int i;

    void print(int i){
        if(i!= 1) return;
        cout << "i: " << i << " rIndex: " << rIndex << " cIndex: " << cIndex << " rcLenth: " << rcLenth << endl;
        cout << "rowArr: " << rowArr[0] << " " << rowArr[1] << endl;
        cout << "colArr: " << colArr[0] << " " << colArr[1] << endl;
    }
} PartialMatrix;

// The thread function
void* rc_Multiplication(void* arg) {
    PartialMatrix* p = (PartialMatrix*)arg;

    int r = p->rIndex, c = p->cIndex, len = p->rcLenth;
    int* rowArr = p->rowArr, *colArr = p->colArr;
    int** RESULT_MATRIX = p->FINAL_RESULT;
    int count = p->i;

    p->print(count);

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

int main() {
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // The two matrix to be multiplied and the place to store the answer
    int M1[2][2] = {{1, 2}, {3, 4}};
    int M2[2][2] = {{5, 6}, {7, 8}};
    int tmp1[] = {M2[0][0], M2[1][0]};
    int tmp2[] = {M2[0][1], M2[1][1]};
    res = new int*[2];
    res[0] = new int[2];
    res[1] = new int[2];
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
            res[i][j] = 0;
        }
    }
    
    PartialMatrix p[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    // Create threads
    for(int i = 0; i < NUM_THREADS; i++){
        if(i<2){
            p[i].rIndex = 0;
            p[i].rowArr = M1[0];
        } else {
            p[i].rIndex = 1;
            p[i].rowArr = M1[1];
        }
        if(i%2 == 0){
            p[i].cIndex = 0;
            p[i].colArr = tmp1;
        } else {
            p[i].cIndex = 1;
            p[i].colArr = tmp2;
        }
        p[i].rcLenth = 2;
        p[i].FINAL_RESULT = res;
        p[i].i = i;
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

    return 0;
}