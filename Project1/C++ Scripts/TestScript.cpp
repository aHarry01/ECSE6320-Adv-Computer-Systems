#include <iostream>
#include <chrono>
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
  long long sum1 = 0;
  for(int i = 0; i < C; i++) {
    for(int j = 0; j < R; j++) {
      sum1 += grid[j][i];
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


  auto start_method1 = chrono::high_resolution_clock::now();
  for(int i = 0; i < 100; i++) {
    method1(grid, R, C);
  }
  auto stop_method1 = chrono::high_resolution_clock::now();
  auto duration_method1 = chrono::duration_cast<std::chrono::milliseconds>(stop_method1 - start_method1).count();

  auto start_method2 = chrono::high_resolution_clock::now();
  for(int i = 0; i < 100; i++) {
    method2(grid, R, C);
  }
  auto stop_method2 = chrono::high_resolution_clock::now();
  auto duration_method2 = chrono::duration_cast<std::chrono::milliseconds>(stop_method2 - start_method2).count();

  cout << "method1 took " << duration_method1 << " milliseconds" << std::endl;
  cout << "method2 took " << duration_method2 << " milliseconds" << std::endl;

  return 0;  
}