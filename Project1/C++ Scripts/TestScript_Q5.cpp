#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <chrono>
using namespace std;

const string FILENAME = "Q5_Input.txt";

int readMethod1(int bufferSize){
    ifstream file(FILENAME);
    char buffer[bufferSize];
    streampos pos;

    int sum = 0;
    while (file.read(buffer, bufferSize)) {
        pos = file.tellg();
        file.seekg(0, ios::cur); 
        sum += 1;
    }
    file.close();
    return sum;
}

int readMethod2(int bufferSize){
    ifstream file(FILENAME);
    char buffer[bufferSize];
    int sum = 0;
    streampos pos;

    while(file.read(buffer, bufferSize)){
        pos = file.tellg();
        file.seekg(bufferSize, ios::cur);
        sum += 1;
    }
    file.close();
    file.open(FILENAME);    //For some reason, this is necessary to reset the file position
    file.seekg(bufferSize, ios::beg);
    while(file.read(buffer, bufferSize)){
        pos = file.tellg();
        file.seekg(bufferSize, ios::cur);
        sum += 1;
    }
    file.close();
    return sum;
}

int main() {

    for(int i = 100; i <= 100000; i *= 10){
        auto start = chrono::high_resolution_clock::now();
        long s = 0;
        for(int j = 0; j < 50; j++)    readMethod1(i);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << "Buffer Size: " << i << " Time_1: " << duration << endl;

        start = chrono::high_resolution_clock::now();
        s = 0;
        for(int j = 0; j < 50; j++)    readMethod2(i);
        end = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << "Buffer Size: " << i << " Time_2: " << duration << endl;
    }
    return 0;

}
