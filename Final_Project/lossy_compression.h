#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include "pthread.h"

using namespace std;

void read_uncompressed(const string& filename, vector<vector<vector<uint8_t>>>& data);
void lossy_compression(vector<vector<vector<uint8_t>>>& data, int num_threads);