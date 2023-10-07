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
    int* rowArr;
    int* colArr;
    int rIndex, cIndex, rcLenth;
    int** FINAL_RESULT;
    int i;

    void print( ){
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

int** readMatrix(string fileName){
    ifstream inf;
    string line;
    inf.open(fileName);
    int n;
    vector<vector<int>> content;
    while(getline(inf, line)){
        vector<int> tmp;
        stringstream iss(line);
        while (iss >> n)
        {
            tmp.push_back(n);
        }
        content.push_back(tmp);
    }
    inf.close();

    n = content.size();
    int m = content[0].size();
    int** res = new int*[n];
    for(int i = 0; i < n; i++){
        res[i] = new int[m];
        for(int j = 0; j < m; j++){
            res[i][j] = content[i][j];
        }
    }
    return res;
}

int main(int argc, char* argv[]) {
    // argv Parameters: [1]: number of threads; [2]: Matrix1; [3]: Matrix2
    if (argc != 4){
        cout << "Usage: ./rc_Multiplication [number of threads] [Matrix1] [Matrix2]" << endl;
        return 0;
    }
    const int NUM_THREADS = stoi(argv[1]);
    int** M1 = readMatrix(argv[2]);
    int** M2 = readMatrix(argv[3]);

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // The two matrix to be multiplied and the place to store the answer
    int tmp1[] = {M2[0][0], M2[1][0]};
    int tmp2[] = {M2[0][1], M2[1][1]};
    int** res;
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
    
    delete res[0];
    delete res[1];
    delete M1[0];
    delete M1[1];
    delete M2[0];
    delete M2[1];
    return 0;
}