# Project 2

## Running the code

All implementations are merged together in main.cpp which can be compiled with
```
g++ -pthread -march=native main.cpp -o main.o
```
The command line usage is:
```
./main.o matrix1_file matrix2_file use_threads use_simd use_cache_miss num_threads output_file
```
* matrix1_file = name of file containing matrix1
* matrix2_file = name of file containing matrix2 
* use_threading = use multithreading optimization (0 or 1)
* use_simd = use SIMD optimization (0 or 1)
* use_cache_miss = use restructuring of data access patterns to minimize cache miss rate (0 or 1)
* num_threads = number of threads to use (only applicable when use_threading = 1)
* output_file = name of output file

For example, to run the 4x4 test case with both 4 threads and using SIMD instructions:
```
./main.o TestData/M4x4.txt TestData/M4x4.txt 1 1 0 4 output.txt
```
