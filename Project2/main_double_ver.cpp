// Merged all implementations together

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

#include <pthread.h>
#include <immintrin.h>

using namespace std;

struct Matrix{
    double** data;
    int height;
    int width;
};

typedef struct PartialMatrix{
// Only one col and one row
public:
    double** rows;
    double** M2T;
    int rIndex, rLength, cLength;
    double** FINAL_RESULT;
} PartialMatrix;

// A mutex to protect shared data
pthread_mutex_t mutex;


// ---- I/O and helper functions ----

// read matrix from file
double** readMatrix(const string &fileName, int &h, int &w){
    ifstream inf;
    string line;
    int i = 0, j;
    double n;

    inf.open(fileName);
    getline(inf, line);
    stringstream iss(line);
    iss >> w >> h;

    double** res = new double*[h];
    for(int i = 0; i < h; i++){
        res[i] = new double[w];
    }
    
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

//write matrix to file
void saveMatrix(const string &fileName, Matrix& M){
    ofstream outfile;
    outfile.open(fileName);

    for (int i = 0; i < M.height; i++){
        for (int j = 0; j < M.width; j++){
            outfile << M.data[i][j] << " ";
        }
        outfile << endl;
    }
    outfile.close();
}


// transpose matrix M
void transposeMatrix(Matrix &M){
    //Transpose a matrix
    double** T = new double*[M.height];
    for(int i = 0; i < M.width; i++){
        T[i] = new double[M.height];
        for(int j = 0; j < M.height; j++){
            T[i][j] = M.data[j][i];
        }
    }
    //Free M
    for(int i = 0; i < M.height; i++){
        delete[] M.data[i];
    }
    delete[] M.data;

    // replace M.data with transpose matrix and swap height/width variables
    int tmp = M.height;
    M.height = M.width;
    M.width = tmp;
    M.data = T;
}


// ---- SIMD functions ----
void simd_matrix_multiply(Matrix &m1, Matrix &m2, double** result, bool m2_is_transpose){
	
	int num_full_ops = m2.width / 8; // number of full 8 word vectorized operations needed per row
	// if the number of elements in 1 row of m1 isn't a multiple of 8,
	// then need to do a partial vector operation at the end
	int leftover = m2.width % 8; // number of "leftover" bytes at the end of each output row
	int mask_vals[8] = {0};
	for (int i = 0; i < leftover; i++){
		mask_vals[i] = 0xFFFFFFFF;
	}
	__m256i mask = _mm256_set_epi32(mask_vals[7], mask_vals[6], mask_vals[5], mask_vals[4], 
									mask_vals[3], mask_vals[2], mask_vals[1], mask_vals[0]);

	double temp[8] = {0};
	__m256d fmadd_result;
	__m256d col;
	__m256d row;

    if (!m2_is_transpose){
        transposeMatrix(m2);
    }

	// Loop through each location in the output array and calculate it
	for(int r1 = 0; r1 < m1.height; r1++){	
		
		for(int c2 = 0; c2 < m2.width; c2++){
			fmadd_result = _mm256_setzero_pd();
			
			// use SIMD instructions to do the multiply & add operations
			// each vectorized instruction can multiply/add 8 4-byte words 
			for (int i = 0; i < num_full_ops; i++){
				row = _mm256_loadu_pd(&m1.data[r1][8*i]);
                col = _mm256_loadu_pd(&m2.data[c2][8*i]);
				fmadd_result = _mm256_fmadd_pd(row, col, fmadd_result);
			}

			_mm256_storeu_pd(temp, fmadd_result);
			// accumulate all the multiplications into the result
			result[r1][c2] = 0;
			for (int i = 0; i < 8; i ++){
				result[r1][c2] += temp[i];
			}	

			fmadd_result = _mm256_setzero_pd();

			// take care of any elements beyond the multiple of 8
			if (leftover != 0){
				row = _mm256_loadu_pd(&m1.data[r1][m1.width-leftover]);
				col = _mm256_loadu_pd(&m2.data[c2][m2.width-leftover]);
                fmadd_result = _mm256_fmadd_pd(row, col, fmadd_result);
				_mm256_storeu_pd(temp, fmadd_result);
				for (int i = 0; i < leftover; i++){
					result[r1][c2] += temp[i];     
				}

			}
		}
	}
}

// ---- Minimize cache miss rate ----

void cache_opt_matrix_multiply(Matrix &m1, Matrix &m2, double** result){
    // transpose m2 first so that the columns become rows and are close together in memory
    // cache can now take advantage of spatial locality for m2 as well as m1
	transposeMatrix(m2); 

	for(int r1 = 0; r1 < m1.height; r1++){
		for(int c2 = 0; c2 < m2.width; c2++){
			result[r1][c2] = 0;
			for(int i = 0; i < m1.width; i++){
				result[r1][c2] += m1.data[r1][i]*m2.data[c2][i];
			}
		}
	}
}

// ---- Multithreading functions ----
// The basic thread function
void* rc_Multiplication(void* arg) {
    PartialMatrix* p = (PartialMatrix*)arg;
    int rIndex = p->rIndex, rLength = p->rLength, cLength = p->cLength;
    double** rows = p->rows, **M2T = p->M2T, **FINAL_RESULT = p->FINAL_RESULT;

    double partialRes[rLength][cLength];
    for(int i = 0; i < rLength; i++){
        for(int j = 0; j < cLength; j++){
            double tmp = 0;
            for(int k = 0; k < cLength; k++){
                tmp += rows[i][k] * M2T[j][k];
            }
            partialRes[i][j] = tmp;
        }
    }

    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&mutex);
    // Access shared data
    for(int i = 0, r = rIndex; i < rLength; i++, r++){
        for(int j = 0; j < cLength; j++){
            FINAL_RESULT[r][j] = partialRes[i][j];
        }
    }
    // Unlock the mutex
    pthread_mutex_unlock(&mutex); 
    return NULL;
}

// The thread function that uses SIMD instructions
void* rc_SIMD_Multiplication(void* arg) {
    PartialMatrix* p = (PartialMatrix*)arg;
    int rIndex = p->rIndex, rLength = p->rLength, cLength = p->cLength;
    double** rows = p->rows, **M2T = p->M2T, **FINAL_RESULT = p->FINAL_RESULT;

    double** partialRes = new double*[rLength];
    for(int i = 0; i < rLength; i++){
        partialRes[i] = new double[cLength]();
    }
    Matrix m1 = {rows, rLength, cLength};
    Matrix m2T = {M2T, cLength, cLength};

    // use SIMD matrix mult method on this sub-matrix
    simd_matrix_multiply(m1, m2T, partialRes, true);

    // Lock the mutex before accessing shared data
    pthread_mutex_lock(&mutex);
    // Access shared data
    for(int i = 0, r = rIndex; i < rLength; i++, r++){
        for(int j = 0; j < cLength; j++){
            FINAL_RESULT[r][j] = partialRes[i][j];
        }
    }
    // Unlock the mutex
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < rLength; i++){
        delete[] partialRes[i];
    }
    delete[] partialRes;

    return NULL;
}

// Threading matrix multiply with the option to use SIMD instructions as well
void threading_matrix_mutliply(Matrix &m1, Matrix &m2, double** result, const int NUM_THREADS, bool use_SIMD){
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    transposeMatrix(m2);

    //Initialize all partial matrices
    int rowsRerThread = m1.height/NUM_THREADS; // The task was to multiply big matrices, so we assume that r1 >> NUM_THREADS
    pthread_t threads[NUM_THREADS];
    PartialMatrix p[NUM_THREADS];
    for (int i = 0, r = 0; i < NUM_THREADS; i++, r += rowsRerThread) {
        p[i].rows = &m1.data[r];
        p[i].M2T = m2.data;
        p[i].rIndex = r;
        p[i].rLength = i == NUM_THREADS-1 ? m1.height-r : rowsRerThread;
        p[i].cLength = m2.width;
        p[i].FINAL_RESULT = result;
    }

    // Create threads
    if (use_SIMD){
        for(int i = 0; i < NUM_THREADS; i++){
            pthread_create(&threads[i], NULL, rc_SIMD_Multiplication, &p[i]);
        }
    } else {
        for(int i = 0; i < NUM_THREADS; i++){
            pthread_create(&threads[i], NULL, rc_Multiplication, &p[i]);
        }
    }


    // Wait for threads to finish
    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

}

// ---- Basic multiply function ---

// Basic naive implementation that makes no attempt to be efficient
// this is the baseline to measure  against
void basic_matrix_multiply(Matrix &m1, Matrix &m2, double** result){
	// Loop through each location in the output array and calculate it
	for(int r1 = 0; r1 < m1.height; r1++){
		for(int c2 = 0; c2 < m2.width; c2++){
			result[r1][c2] = 0;
			for(int i = 0; i < m1.width; i++){
				result[r1][c2] += m1.data[r1][i]*m2.data[i][c2];
			}
		}
	}
}


int main(int argc, char* argv[]){
    // argv params: [1] Matrix1 file, [2] Matrix2 file
    //              [3] use multithreading (0 or 1), [4] use SIMD (0 or 1), [5] minimize cache miss rate (0 or 1)
    //              [6] number of threads, [7] output file
    if (argc != 8){
        std::cout << "Wrong number of instructions" << std::endl;
        //Output an error about number of arguments or can't multiply matrices
        return 0;
    }
    const int NUM_THREADS = stoi(argv[6]);
    const int use_Threads = stoi(argv[3]);
    const int use_SIMD = stoi(argv[4]);
    const int use_CacheMiss = stoi(argv[5]);

    // Read in matrices
    int r1, c1, r2, c2;
    double** M1_data = readMatrix(argv[1], r1, c1);
    Matrix M1 = {M1_data, r1, c1};
    double** M2_data = readMatrix(argv[2], r2, c2);
    Matrix M2 = {M2_data, r2, c2};

    // for(int i = 0; i < r1; i++){
    //     for(int j = 0; j < c1; j++){
    //         cout << M1.data[i][j] << " ";
    //     }
    //     cout << endl;
    // }
    
    // initialize results matrix
    double** res;
    res = new double*[r1];
    for(int i = 0; i < r1; i++){
        res[i] = new double[c2]();
    }
    cout << "Matrices initialized and read in" << endl;

    // multiply matrices
    auto start = std::chrono::high_resolution_clock::now();
    if (!use_SIMD && !use_Threads && !use_CacheMiss){
        basic_matrix_multiply(M1, M2, res);
    } else if (use_Threads){ // threading or threading & SIMD
        threading_matrix_mutliply(M1, M2, res, NUM_THREADS, (bool) use_SIMD);
    } else if (use_SIMD){ // just SIMD
        simd_matrix_multiply(M1, M2, res, false);
    } else if (use_CacheMiss){ // just cache miss
        cache_opt_matrix_multiply(M1, M2, res);
    }

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
    cout << "Matrix multiply complete" << endl;
	cout << "Took " << elapsed.count() << " seconds" << endl;

    // save result to file
    Matrix result = {res, M1.height, M2.width};
    saveMatrix(argv[7], result);

    //delete memory
    for(int i = 0; i < M1.height; i++){
        delete[] M1.data[i];
    }
    delete[] M1.data;

    for(int i = 0; i < M2.height; i++){
        delete[] M2.data[i];
    }
    delete[] M2.data;

    for(int i = 0; i < result.height; i++){
        delete[] result.data[i];
    }
    delete[] result.data;


}