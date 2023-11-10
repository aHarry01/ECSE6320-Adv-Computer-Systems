#ifndef ENCODE_H_
#define ENCODE_H_
#include <string>
#include <cassert>
#include <vector>
#include <iostream>
#include <fstream>
#include "zlib.h"
using namespace std;

const long int CHUNK = {65536};

uint64_t ColumnHash(string const& word);
uint64_t Hash(string const& word);
int Compress(string dataPath, string compressedDataPath);
vector<uint64_t> Decompress(string compressedDataPath, unsigned int length);
#endif