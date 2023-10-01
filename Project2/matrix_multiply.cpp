
#include <immintrin.h>
#include <iostream>
#include <string>
#include <cstring>
#include <chrono>


// WIDTH1 must equal HEIGHT2 in order for these matrices to be multiplied
#define WIDTH1 1025   // width for m1
#define HEIGHT1 1025  // height for m1

#define WIDTH2 1025 // width for m2
#define HEIGHT2 1025 // height for m2


// Put some data in the m1 test array
void init_test_data1(float test_data[HEIGHT1][WIDTH1]){
	for (int r = 0; r < HEIGHT1; r++){
		for(int c = 0; c < WIDTH1; c++){
			test_data[r][c] = r*HEIGHT1 + c;
			//std::cout << test_data[r][c] << " ";
		}
		//std::cout << std::endl;
	}
	//std::cout << std::endl;
}
// Put some data in the m2 test array
void init_test_data2(float test_data[HEIGHT2][WIDTH2]){
	for (int r = 0; r < HEIGHT2; r++){
		for(int c = 0; c < WIDTH2; c++){
			test_data[r][c] = r*HEIGHT2 + c;
			//std::cout << test_data[r][c] << " ";
		}
		//std::cout << std::endl;
	}
	//std::cout << std::endl;
}


// TODO: check result of basic_matrix_multiply with simd_matrix_multiply
//       for large test cases
//bool check_result(){
	
//}


// Basic naive implementation that makes no attempt to be efficient
// this is the baseline to measure speed up against
void basic_matrix_multiply(float m1[HEIGHT1][WIDTH1], float m2[HEIGHT2][WIDTH2], float result[HEIGHT1][WIDTH2]){
	// Loop through each location in the output array and calculate it
	for(int r1 = 0; r1 < HEIGHT1; r1++){
		for(int c2 = 0; c2 < WIDTH2; c2++){
			result[r1][c2] = 0;
			for(int i = 0; i < WIDTH1; i++){
				result[r1][c2] += m1[r1][i]*m2[i][c2];
			}
		}
	}
}


// does m1 x m2
void simd_matrix_multiply(float m1[HEIGHT1][WIDTH1], float m2[HEIGHT2][WIDTH2], float result[HEIGHT1][WIDTH2]){
	
	int num_full_ops = WIDTH2 / 8; // number of full 8 word vectorized operations needed per operation
	// if the number of elements in 1 row of m1 isn't a multiple of 8,
	// then need to do a partial vector operation at the end
	int leftover = WIDTH2 % 8; // number of "leftover" bytes at the end of each output row
	int mask_vals[8] = {0};
	for (int i = 0; i < leftover; i++){
		mask_vals[i] = 0xFFFFFFFF;
	}
	__m256i mask = _mm256_set_epi32(mask_vals[7], mask_vals[6], mask_vals[5], mask_vals[4], 
									mask_vals[3], mask_vals[2], mask_vals[1], mask_vals[0]);

	// indices to gather data from the columns of the matrix
	__m256i gather_indices = _mm256_set_epi32(7*WIDTH2, 6*WIDTH2, 5*WIDTH2, 4*WIDTH2,
	 										  3*WIDTH2, 2*WIDTH2, 1*WIDTH2, 0*WIDTH2);
	__m256i gather_leftover_indicies = _mm256_set_epi32(mask_vals[7] & (7*WIDTH2),
														mask_vals[6] & (6*WIDTH2),
														mask_vals[5] & (5*WIDTH2), 
														mask_vals[4] & (4*WIDTH2),
				 										mask_vals[3] & (3*WIDTH2), 
				 										mask_vals[2] & (2*WIDTH2), 
				 										mask_vals[1] & (1*WIDTH2),
				 										mask_vals[0] & (0*WIDTH2));
	float temp[8] = {0};
	__m256 fmadd_result;
	__m256 col;
	__m256 row;

	//TODO possibly transpose m2 before so that we don't need to use i32gather each time to load things
	// check if this is actually a performance improvement though

	// Loop through each location in the output array and calculate it
	for(int r1 = 0; r1 < HEIGHT1; r1++){	
		
		for(int c2 = 0; c2 < WIDTH2; c2++){
			fmadd_result = _mm256_set_ps(0,0,0,0,0,0,0,0);
			
			// use SIMD instructions to do the multiply & add operations
			// each vectorized instruction can multiply/add 8 4-byte words 
			for (int i = 0; i < num_full_ops; i++){
				row = _mm256_loadu_ps(&m1[r1][8*i]);
				col = _mm256_i32gather_ps(&m2[8*i][c2], gather_indices, 4);
				fmadd_result = _mm256_fmadd_ps(row, col, fmadd_result);
			}

			_mm256_storeu_ps(temp, fmadd_result);
			// accumulate all the multiplications into the result
			result[r1][c2] = 0;
			for (int i = 0; i < 8; i ++){
				result[r1][c2] += temp[i];
			}	

			fmadd_result = _mm256_set_ps(0,0,0,0,0,0,0,0);

			// TODO: is this faster than not using fmadd for this part???
			// take care of any elements beyond the multiple of 8
			if (leftover != 0){
				row = _mm256_loadu_ps(&m1[r1][WIDTH1-leftover]);
				col = _mm256_i32gather_ps(&m2[HEIGHT2-leftover][c2], gather_leftover_indicies, 4);
				fmadd_result = _mm256_fmadd_ps(row, col, fmadd_result);
				_mm256_storeu_ps(temp, fmadd_result);
				for (int i = 0; i < leftover; i++){
					result[r1][c2] += temp[i];
				}
			}
		}
	}
}


int main(int argc, char *argv[]){

	float (*m1)[WIDTH1] = (float (*)[WIDTH1]) malloc(sizeof(float[HEIGHT1][WIDTH1])); //(float (*)[WIDTH1]) aligned_alloc(32, sizeof(float[HEIGHT1][WIDTH1]));
	float (*m2)[WIDTH2] = (float (*)[WIDTH2]) malloc(sizeof(float[HEIGHT2][WIDTH2])); //(float (*)[WIDTH2]) aligned_alloc(32, sizeof(float[HEIGHT2][WIDTH2]));
	init_test_data1(m1);
	init_test_data2(m2);

	// --- basic implementation ----------------------------------------------------------
	float (*result)[WIDTH2] = (float (*)[WIDTH2]) malloc(sizeof(float[HEIGHT1][WIDTH2]));
    auto start = std::chrono::high_resolution_clock::now();
    
	basic_matrix_multiply(m1, m2, result);

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	std::cout << "Basic multiply took " << elapsed.count() << " seconds" << std::endl;

	free(result);


    // --- SIMD implementation -----------------------------------------------------------
	float (*result2)[WIDTH2] = (float (*)[WIDTH2]) malloc(sizeof(float[HEIGHT1][WIDTH2]));
	start = std::chrono::high_resolution_clock::now();

    simd_matrix_multiply(m1, m2, result2);

	finish = std::chrono::high_resolution_clock::now();
	elapsed = finish - start;
    std::cout << "SIMD multiply took " << elapsed.count() << " seconds" << std::endl;

    free(result2);

    //	for (int r = 0; r < HEIGHT1; r++){
	// 	for(int c = 0; c < WIDTH2; c++){
	// 		std::cout << result[r][c] << " ";
	// 	}
	// 	std::cout << std::endl;
	// }

	free(m1);
	free(m2);
}