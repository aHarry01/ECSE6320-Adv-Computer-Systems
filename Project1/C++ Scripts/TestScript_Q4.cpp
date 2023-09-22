#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
using namespace std;

const int R = 2000;
const int C = 1024;

void method1(int grid[][C], int R, int C){
  //Method 1

  long long sum = 0;
  for(int i = 0; i < R; i++) {
    for(int j = 0; j < C; j++) {
      sum += grid[i][j];
    }
  }
}

void method2(int grid[][C], int R, int C){
  //Method 2
  long long sum = 0;
  for(int i = 0; i < C; i++) {
    for(int j = 0; j < R; j++) {
      sum += grid[j][i];
    }
  }
}

void method3(int grid[][C], int R, int C){
  //Method 3
  long long sum = 0;

  for(int i = 0; i < R/2; i++) {
    for(int j = 0; j < C; j++) {
      sum += grid[i][j];
    }
  }

  for(int i = 0; i < C; i++) {
    for(int j = R/2; j < R; j++) {
      sum += grid[j][i];
    }
  }
}

int main() {
  //Init the grid
  int grid[R][C];
  for(int i = 0; i < R; i++) {
    for(int j = 0; j < C; j++) {
      grid[i][j] = i+j;
    }
  }

  //Test the methods' times
  int testLoops = 1;
  vector<int> method1Times, method2Times, method3Times;
  for(int loop = testLoops; loop <= 1000; loop*=10) {
    auto start_method1 = chrono::high_resolution_clock::now();
    for(int i = 0; i < loop; i++) {
      method1(grid, R, C);
    }
    auto stop_method1 = chrono::high_resolution_clock::now();
    auto duration_method1 = chrono::duration_cast<std::chrono::milliseconds>(stop_method1 - start_method1).count();
    method1Times.push_back(duration_method1);

    auto start_method2 = chrono::high_resolution_clock::now();
    for(int i = 0; i < loop; i++) {
      method2(grid, R, C);
    }
    auto stop_method2 = chrono::high_resolution_clock::now();
    auto duration_method2 = chrono::duration_cast<std::chrono::milliseconds>(stop_method2 - start_method2).count();
    method2Times.push_back(duration_method2);

    auto start_method3 = chrono::high_resolution_clock::now();
    for(int i = 0; i < loop; i++) {
      method3(grid, R, C);
    }
    auto stop_method3 = chrono::high_resolution_clock::now();
    auto duration_method3 = chrono::duration_cast<std::chrono::milliseconds>(stop_method3 - start_method3).count();
    method3Times.push_back(duration_method3);
  }

  //Export the times to csv
  std::ofstream outFile;
  outFile.open("../Problem4_Times.csv");
  outFile << "Method_1_Time,Method_2_Time\n";
  // Write data
  for(int i=0; i<5; i++) {
    outFile << method1Times[i] << "," << method2Times[i] << "," << method3Times[i] << "\n";
  }
  outFile.close();
  return 0;  
}