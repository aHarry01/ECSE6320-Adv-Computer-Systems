#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
using namespace std;



int main(int argc, char* argv[]){
    // argv Parameters: [1]: Img folder path, [2]: Output folder path, [3]: Compression instruction
    // To test, we use BMP image files, and compress it.
    string folderPath = argv[1];
    // string outputPath = argv[2];
    // string instruction = argv[3];

    DIR *p_dir;
    vector<string> fileList;

    p_dir = opendir(folderPath.append("/").c_str());
    if( p_dir == NULL)  cerr<< "can't open :" << folderPath << endl;

    struct dirent *p_dirent;
    while ( p_dirent = readdir(p_dir))
    {   
        string tmpFileName = p_dirent->d_name;
        if( tmpFileName == "." || tmpFileName == "..")
        {
            continue;
        }
        string extension = tmpFileName.substr(tmpFileName.length() - 4);
        if( extension == ".bmp")
        {
            //Perform the compress using the file name.
            fileList.push_back(tmpFileName);
        }
    }
    closedir(p_dir);

    for(auto f : fileList){
        cout << f << endl;
    }
}
