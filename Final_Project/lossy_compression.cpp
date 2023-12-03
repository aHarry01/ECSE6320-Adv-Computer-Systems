// Lossy compression (based on JPEG)

// TODO: 
// test with 1 thread, different numbers of threads, with/without SIMD
// try some cache optimization or something???
// can also compare compression quality
// combine lossy with lossless to determine maximum compression??

// JPEG compression:
// Convert RGB to Y/Cb/Cr (Y = brightness, Cb/Cr are chrominance split into blue/red)
// can downsample the Cb/Cr components (but don't have to...)
// DCT to convert to frequency domain (use SIMD instructions)
// Quantization (based on matrix, higher freqs can tolerate more quantization error)
// this makes the data more compressible by making it more uniform/smaller numbers

#include "lossy_compression.h"

pthread_mutex_t mutexLossyComp;  // A mutex to protect shared data
vector<vector<vector<uint8_t>>>* data_ptr;
bool use_simd = false;

typedef struct ThreadInfo{
public:
    unsigned int rowStart, rowNum;
} ThreadInfo;

// DCT constants, pre-computed at the beginning of the compression
float cosines[8][8];
//float coeffs[8][8]; 
float zero_coeff = 1/sqrt(2); // coefficient when either i or j is 0

// Read in an uncompressed bitmap file
// BMP files must be in uncompressed mode and represent pixels using RGB data w/ 24 bits per pixel
void read_uncompressed(const string& filename, vector<vector<vector<uint8_t>>>& data){

    // Read the file in
    ifstream file(filename);
    vector<uint8_t> buffer;
    if (!file) {
        cerr << "ERROR: Couldn't open input file " << filename << endl;
        return;
    }
    file.seekg(0,std::ios::end);
    streampos length = file.tellg();
    file.seekg(0,std::ios::beg);
    buffer.resize(length);
    file.read((char*) (&buffer[0]),length);
    file.close();
    
    // make sure this is a BMP file (first 2 chars are 'B' and 'M')
    if ((char) buffer[0] != 'B' || (char) buffer[1] != 'M'){
        cerr << "ERROR: Input file is not a BMP" << endl;
        return;
    }

    // check the header to make sure the right settings are set
    int header_sz = (buffer[17] << 24) | (buffer[16] << 16) | (buffer[15] << 8) | buffer[14];
    int32_t width, height = 0;
    if (header_sz < 40){ // BITMAPCOREHEADER
        width = (buffer[19] << 8) | buffer[18];
        height = (buffer[21] << 8) | buffer[20];
        if (buffer[24] << 8 != 24) { // make sure there are 24 bits per pixel
            cout << "ERROR: input file should have 24 bits per pixel" << endl;
            return;
        }
    } else{ // BITMAPINFOHEADER 
        width = (buffer[21] << 24) | (buffer[20] << 16) | (buffer[19] << 8) | buffer[18];
        height = (buffer[25] << 24) | (buffer[24] << 16) | (buffer[23] << 8) | buffer[22];
        if (buffer[28] != 24) { // make sure there are 24 bits per pixel
            cout << "ERROR: input file must have 24 bits per pixel" << endl;
            return;
        }
        if (buffer[30] != 0 || buffer[31] != 0 || buffer[32] != 0 || buffer[33] != 0){
            cout << "ERROR: input file cannot be compressed" << endl;
            return;
        }
    }

    // read in the image data
    data.resize(height, vector<vector<uint8_t>>(width, vector<uint8_t>(3)));
    int data_addr = (buffer[13] << 24) | (buffer[12] << 16) | (buffer[11] << 8) | buffer[10];

    // Copy the data in row-by-row
    // note that rows are stored in reverse order in BMP files
    // and colors are in blue, green, red order
    for (int r = height-1; r >= 0; r--){
        for (int c = 0; c < width; c++){
            data[r][c][0] = buffer[data_addr+2]; // R
            data[r][c][1] = buffer[data_addr+1]; // G
            data[r][c][2] = buffer[data_addr]; // B
            data_addr += 3;
        }
        // padded so that the row is a multiple of 4 bytes
        data_addr += (width*3 % 4);
    }
}

// illuminance/chrominance quantization tables taken from Annex K of the JPEG spec 
// https://www.w3.org/Graphics/JPEG/itu-t81.pdf
// higher frequencies can be more quantized, and chrominance in general can be more quantized than illuminance
uint8_t luminance_quantization[8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                                        {12, 12, 14, 19, 26, 58, 60, 55},
                                        {14, 13, 16, 24, 40, 57, 69, 56},
                                        {14, 17, 22, 29, 51, 87, 80, 62},
                                        {18, 22, 37, 56, 68, 109, 103, 77},
                                        {24, 35, 55, 64, 81, 104, 113, 92},
                                        {49, 64, 78, 87, 103, 121, 120, 101},
                                        {72, 92, 95, 98, 112, 100, 103, 99}};

uint8_t chrominance_quantization[8][8] = {{17, 18, 24, 47, 99, 99, 99, 99},
                                          {18, 21, 26, 66, 99, 99, 99, 99},
                                          {24, 26, 56, 99, 99, 99, 99, 99},
                                          {47, 66, 99, 99, 99, 99, 99, 99},
                                          {99, 99, 99, 99, 99, 99, 99, 99},
                                          {99, 99, 99, 99, 99, 99, 99, 99},
                                          {99, 99, 99, 99, 99, 99, 99, 99},
                                          {99, 99, 99, 99, 99, 99, 99, 99}};

void precompute_DCT_constants(){
    for (int x = 0; x < 8; x++){
        for(int i = 0; i < 8; i++){
            cosines[x][i] = cos(((2*x + 1)*i*M_PI)/16);
            cout << cosines[x][i] << " ";
        }
        cout << endl;
    }
}

void DCT_test(){
    int test_input[8][8] = {
        {140,144,147,140,140,155,179,175},
        {144,152,140,147,140,148,167,179},
        {152,155,136,167,163,162,152,172},
        {168,145,156,160,152,155,136,160},
        {162,148,156,148,140,136,147,162},
        {147,167,140,155,155,140,136,162},
        {136,156,123,167,162,144,140,147},
        {148,155,136,155,152,147,147,136}
    };
    int test_input_old[8][8] = {
        {154, 123, 123, 123, 123, 123, 123, 136},
        {192, 180, 136, 154, 154, 154, 136, 110},
        {254, 198, 154, 154, 180, 154, 123, 123},
        {239, 180, 136, 180, 180, 166, 123, 123},
        {180, 154, 136, 167, 166, 149, 136, 136},
        {128, 136, 123, 136, 154, 180, 198, 154},
        {123, 105, 110, 149, 136, 136, 180, 166},
        {110, 136, 123, 123, 123, 136, 154, 136}
    };
    int16_t test_output[8][8];
    // do the DCT on this block
    for (int i = 0; i < 8; i++){ // i is row
        for(int j = 0; j < 8; j++){ // j is column
            float temp = 0.0;
            for (int y = 0; y < 8; y++){
                for(int x = 0; x < 8; x++){
                    temp += (test_input[y][x]-128)*cosines[x][j]*cosines[y][i];
                    if (i == 0 && j == 0){
                        cout << y << " " << x << " " << temp << " " << (test_input[y][x]) << endl;
                    }
                }
            }
            if (i == 0) temp *= zero_coeff;
            if (j == 0) temp *= zero_coeff;
            
            test_output[i][j] = (int16_t)(0.25*temp);
            
            cout << (int)test_output[i][j] << " ";
        }
        cout << endl;
    }  
}

// DCT on an 8x8 block with no optimizations
void basic_DCT(int r, int c, uint8_t tmp_y[8][8], uint8_t tmp_cb[8][8], uint8_t tmp_cr[8][8]){
    for (int i = 0; i < 8; i++){ // i is row
        for(int j = 0; j < 8; j++){ // j is column
            float temp = 0.0;
            for (int y = 0; y < 8; y++){
                for(int x = 0; x < 8; x++){
                    temp += (tmp_y[y][x]-128)*cosines[x][j]*cosines[y][i];
                }
            }
            if (i == 0) temp *= zero_coeff;
            if (j == 0) temp *= zero_coeff;
            
            output_data[r+i][c+j][0] = (int8_t)round(0.25*temp / luminance_quantization[i][j]);
            
            cout << (int)output_data[r+i][c+j][0] << " "; //<< (0.25*temp) / luminance_quantization[i][j] << " ";
        }
        cout << endl;
    }
}


// Each thread processes one section of the input file
void* process_block(void* args){
    ThreadInfo* info = (ThreadInfo*)args;
    cout << info->rowStart << "   " << info->rowNum << endl;

    // TODO: what if the height/width are not a multiple of 8? also, swap these...
    // loop through 8x8 blocks and process each
    for (int c = 0; c < (*data_ptr)[0].size(); c+=8){
        for (int r = info->rowStart; r < info->rowNum; r+=8){
            cout <<"-----------------" << endl;
            if (!use_simd){
                int y, cb, cr = 0;
                uint8_t tmp_y[8][8];
                uint8_t tmp_cb[8][8];
                uint8_t tmp_cr[8][8];

                //TODO: can this be accelerated w/ SIMD?
                for (int i = 0; i < 8; i++){
                    for(int j = 0; j < 8; j++){
                        // convert RGB pixels to Y/Cb/Cr
                        y = 16 + (65.738/256)*((int)(*data_ptr)[r+i][c+j][0]) + (129.057/256)*(*data_ptr)[r+i][c+j][1] + (25.064/256)*(*data_ptr)[r+i][c+j][2];
                        cb = 128 - (37.945/256)*(*data_ptr)[r+i][c+j][0] - (74.494/256)*(*data_ptr)[r+i][c+j][1] + (112.439/256)*(*data_ptr)[r+i][c+j][2];
                        cr = 128 + (112.439/256)*(*data_ptr)[r+i][c+j][0] - (94.154/256)*(*data_ptr)[r+i][c+j][1] - (18.285/256)*(*data_ptr)[r+i][c+j][2];
                        tmp_y[i][j] = y;
                        tmp_cb[i][j] = cb;
                        tmp_cr[i][j] = cr;
                        //cout << (int)(*data_ptr)[r+j][c+i][0] << " " << (int)(*data_ptr)[r+j][c+i][1] << " " << (int)(*data_ptr)[r+j][c+i][2] << endl;
                        //cout << y << " " << cb << " " << cr << endl << endl;
                    }
                }

                // do the DCT & subsequent quantization on this block
                basic_DCT(r, c, tmp_y, tmp_cb, tmp_cr);
            }
            else{
                cout << "SIMD optimization not implemented yet" << endl;
            }

            // after DCT there should be more zeros/repeated values and the result should be a larger compression with the encoding

        }
    }

    return NULL;
}

// lossy compression (JPEG)
void lossy_compression(vector<vector<vector<uint8_t>>>& data, int num_threads){
    cout << data.size() << " x " << data[0].size() << endl;
    output_data.resize(data.size(),vector<vector<int8_t> >(data[0].size(),vector<int8_t>(3)));
    data_ptr = &data;
    // use_simd = ;

    pthread_mutex_init(&mutexLossyComp, NULL);
    vector<pthread_t*> threads;

    // number of rows per thread has to be a multiple of 8
    // since JPEG does the DCT on 8x8 blocks
    int rows_per_thread = data.size()/num_threads;
    rows_per_thread -= rows_per_thread % 8;

    // create the threads, each will process a certain number of rows
    for (int t = 0; t < num_threads-1; t++){
        pthread_t* thread = new pthread_t;

        ThreadInfo* info = new ThreadInfo();
        info->rowStart = t*rows_per_thread;
        info->rowNum = rows_per_thread;

        pthread_create(thread, NULL, process_block, info);
        threads.push_back(thread);
    }
    // the last thread gets all the remaining rows
    pthread_t* thread = new pthread_t;
    ThreadInfo* info = new ThreadInfo();
    info->rowStart = rows_per_thread*(num_threads-1);
    info->rowNum = data.size() - info->rowStart;
    pthread_create(thread, NULL, process_block, info);  
    threads.push_back(thread);

    // wait for threads to finish
    for (unsigned int x = 0; x < threads.size(); x++){
        pthread_join(*threads[x], NULL);
    }

    pthread_mutex_destroy(&mutexLossyComp);
}


// temporary main so I can test lossy compression separate from the rest
// int main(int argc, char* argv[]){

//     // Read in uncompressed image data (BMP)
//     string filename = "test_images/small_test.bmp";
//     vector<vector<vector<uint8_t>>> data;
//     read_uncompressed(filename, data);

//     lossy_compression(data, 2);
// }

