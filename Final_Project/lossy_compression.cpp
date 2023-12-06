// Lossy compression (based on JPEG)

// TODO: 
// test with 1 thread, different numbers of threads, with/without SIMD
// try some cache optimization or something???
// can also compare compression quality
// combine lossy with lossless to determine maximum compression??

#include "lossy_compression.h"
#include "lossy_decompression.h"

pthread_mutex_t mutexLossyComp;  // A mutex to protect shared data
vector<vector<vector<uint8_t>>>* data_ptr; // input data
bool use_simd = false;

typedef struct ThreadInfo{
public:
    unsigned int rowStart, rowNum;
    vector<vector<int8_t>> encoded_y; // list of encoded data for each block
    vector<vector<int8_t>> encoded_cb;
    vector<vector<int8_t>> encoded_cr;
} ThreadInfo;

// DCT constants, pre-computed at the beginning of the compression
float cosines[8][8];
float zero_coeff = 1/sqrt(2); // coefficient when either i or j is 0

float coeff_matrix[8][8]; // only used when using SIMD instructions

// Read in an uncompressed bitmap file
// BMP files must be in uncompressed mode and represent pixels using RGB data w/ 24 bits per pixel
void read_uncompressed(const string& filename, vector<vector<vector<uint8_t>>>& data, vector<uint8_t>& header){

    // Read the file in
    ifstream file(filename);
    vector<uint8_t> buffer;
    if (!file) {
        cout << "ERROR: Couldn't open input file " << filename << endl;
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
        cout << "ERROR: Input file is not a BMP" << endl;
        return;
    }

    // check the header to make sure the right settings are set
    int header_sz = (buffer[17] << 24) | (buffer[16] << 16) | (buffer[15] << 8) | buffer[14];
    int32_t width, height = 0;
    if (header_sz < 40){ // BITMAPCOREHEADER
        width = (buffer[19] << 8) | buffer[18];
        height = (buffer[21] << 8) | buffer[20];
        if (buffer[24] << 8 != 24) { // make sure there are 24 bits per pixel
            cout << "ERROR: input file must have 24 bits per pixel" << endl;
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
        }
    }
}

void precompue_DCT_coeff_matrix(){
    for (int i =0 ; i < 8; i++){
        for (int j =0; j < 8;j++){
            if (i == 0) coeff_matrix[i][j] = 0.35355; 
            else coeff_matrix[i][j] = (1/2.0)*cosines[j][i];
        }
    }
}

// DCT on an 8x8 block with no optimizations
void basic_DCT(uint8_t tmp_y[8][8], uint8_t tmp_cb[8][8], uint8_t tmp_cr[8][8], int8_t out_y[8][8], int8_t out_cb[8][8], int8_t out_cr[8][8]){
    for (int i = 0; i < 8; i++){ // i is row
        for(int j = 0; j < 8; j++){ // j is column
            float temp = 0.0;
            float temp2 = 0.0;
            float temp3 = 0.0;
            for (int y = 0; y < 8; y++){
                for(int x = 0; x < 8; x++){
                    temp += (tmp_y[y][x]-128)*cosines[x][j]*cosines[y][i];
                    temp2 += (tmp_cb[y][x]-128)*cosines[x][j]*cosines[y][i];
                    temp3 += (tmp_cr[y][x]-128)*cosines[x][j]*cosines[y][i];
                }
            }
            if (i == 0){
                temp *= zero_coeff;
                temp2 *= zero_coeff;
                temp3 *= zero_coeff;
            } 
            if (j == 0){
                temp *= zero_coeff;
                temp2 *= zero_coeff;
                temp3 *= zero_coeff;
            } 
            
            out_y[i][j] = (int8_t)round(0.25*temp / luminance_quantization[i][j]);
            out_cb[i][j] = (int8_t)round(0.25*temp2 / chrominance_quantization[i][j]);
            out_cr[i][j] = (int8_t)round(0.25*temp3 / chrominance_quantization[i][j]);
            //cout << (int)out_y[i][j] << " ";
            //cout << (int)out_y[i][j] << " "; //<< (0.25*temp) / luminance_quantization[i][j] << " ";
        }
        //cout << endl;
    }
    //cout << endl;
    // for (int i = 0; i < 8; i++){ // i is row
    //     for(int j = 0; j < 8; j++){ // j is column
    //         cout << (int) out_y[i][j] << " ";
    //     } 
    //     cout << endl;
    // }
    // cout << endl;
}

// DCT on an 8x8 block using SIMD instructions
void SIMD_DCT(uint8_t tmp_y[8][8], uint8_t tmp_cb[8][8], uint8_t tmp_cr[8][8], int8_t out_y[8][8], int8_t out_cb[8][8], int8_t out_cr[8][8]){    
    // first, load our matrix as floats
    __m256 rows_y[8]; __m256 rows_cb[8]; __m256 rows_cr[8];
    __m256i constant_subtract = _mm256_set_epi32(128, 128, 128, 128, 128, 128, 128, 128);
    
    for(int r = 0; r < 8; r++){
        // convert to 32-bit integers, then subtract 128 from each, then convert to floats
        rows_y[r] = _mm256_cvtepi32_ps(
                    _mm256_sub_epi32(
                        _mm256_cvtepu8_epi32(_mm_loadu_si128((__m128i*)(tmp_y[r]))), 
                        constant_subtract
                    ));
        rows_cb[r] = _mm256_cvtepi32_ps(
                     _mm256_sub_epi32(
                        _mm256_cvtepu8_epi32(_mm_loadu_si128((__m128i*)(tmp_cb[r]))), 
                        constant_subtract
                    ));
        rows_cr[r] = _mm256_cvtepi32_ps(
                     _mm256_sub_epi32(
                        _mm256_cvtepu8_epi32(_mm_loadu_si128((__m128i*)(tmp_cr[r]))), 
                        constant_subtract
                    ));
    }

    // the DCT can be done with 2 matrix multiplications:
    //  temp_result = tmp_y*transpose(coeff_matrix)
    //  out_y = coeff_matrix*temp_result

    // temporary variables
    __m256 fmadd_result_y; __m256 fmadd_result_cb;  __m256 fmadd_result_cr;
    __m256 coeff_tmp;
    float temp[8] = {0};
    // stored as the TRANSPOSE of tmp_y*transpose(coeff_matrix)
    float temp_result_y_T[8][8]; float temp_result_cb_T[8][8]; float temp_result_cr_T[8][8]; 

    // first matrix multiplication, temp_result = tmp_y*transpose(coeff_matrix)
    for (int i = 0; i < 8; i++){ // i is row
        for(int j = 0; j < 8; j++){ // j is column
		    fmadd_result_y = _mm256_set_ps(0,0,0,0,0,0,0,0);
            fmadd_result_cb = _mm256_set_ps(0,0,0,0,0,0,0,0);
            fmadd_result_cr = _mm256_set_ps(0,0,0,0,0,0,0,0);
			
			// use SIMD instructions to do the multiply & add operations
			// each vectorized instruction can multiply/add 8 4-byte words 
			// which is all we need since the matrix is always 8x8
            coeff_tmp = _mm256_loadu_ps(coeff_matrix[j]);
            fmadd_result_y = _mm256_fmadd_ps(rows_y[i], coeff_tmp, fmadd_result_y);
			_mm256_storeu_ps(temp, fmadd_result_y);
			// accumulate all the multiplications into the result
			temp_result_y_T[j][i] = 0;
			for (int k = 0; k < 8; k ++){
				temp_result_y_T[j][i] += temp[k];
			}	

            fmadd_result_cb = _mm256_fmadd_ps(rows_cb[i], coeff_tmp, fmadd_result_cb);
			_mm256_storeu_ps(temp, fmadd_result_cb);
			// accumulate all the multiplications into the result
			temp_result_cb_T[j][i] = 0;
			for (int k = 0; k < 8; k ++){
				temp_result_cb_T[j][i] += temp[k];
			}	

            fmadd_result_cr = _mm256_fmadd_ps(rows_cr[i], coeff_tmp, fmadd_result_cr);
			_mm256_storeu_ps(temp, fmadd_result_cr);
			// accumulate all the multiplications into the result
			temp_result_cr_T[j][i] = 0;
			for (int k = 0; k < 8; k ++){
				temp_result_cr_T[j][i] += temp[k];
			}	
        }
    }

    float result = 0.0;
    // second matrix multiplication, out_y = coeff_matrix*temp_result
    for (int i = 0; i < 8; i++){ // i is row
        for(int j = 0; j < 8; j++){ // j is column
		    fmadd_result_y = _mm256_set_ps(0,0,0,0,0,0,0,0);
            fmadd_result_cb = _mm256_set_ps(0,0,0,0,0,0,0,0);
            fmadd_result_cr = _mm256_set_ps(0,0,0,0,0,0,0,0);
			
			// use SIMD instructions to do the multiply & add operations
			// each vectorized instruction can multiply/add 8 4-byte words 
			// which is all we need since the matrix is always 8x8
            coeff_tmp = _mm256_loadu_ps(coeff_matrix[i]);
            fmadd_result_y = _mm256_fmadd_ps(coeff_tmp, _mm256_loadu_ps(temp_result_y_T[j]), fmadd_result_y);
			_mm256_storeu_ps(temp, fmadd_result_y);
			// accumulate all the multiplications into the result
			result = 0.0;
			for (int k = 0; k < 8; k ++){
				result += temp[k];
			}	
            out_y[i][j] = (int8_t)round(result/luminance_quantization[i][j]);

            fmadd_result_cb = _mm256_fmadd_ps(coeff_tmp, _mm256_loadu_ps(temp_result_cb_T[j]), fmadd_result_cb);
			_mm256_storeu_ps(temp, fmadd_result_cb);
			// accumulate all the multiplications into the result
			result = 0.0;
			for (int k = 0; k < 8; k ++){
				result += temp[k];
			}	
            out_cb[i][j] = (int8_t)round(result/chrominance_quantization[i][j]);

            fmadd_result_cr = _mm256_fmadd_ps(coeff_tmp, _mm256_loadu_ps(temp_result_cr_T[j]), fmadd_result_cr);
			_mm256_storeu_ps(temp, fmadd_result_cr);
			// accumulate all the multiplications into the result
			result = 0.0;
			for (int k = 0; k < 8; k ++){
				result += temp[k];
			}	
            out_cr[i][j] = (int8_t)round(result/chrominance_quantization[i][j]);
        }
    }

    // Quantization and convert back to integers
    // for (int i = 0; i < 8; i++){ // i is row
    //     for(int j = 0; j < 8; j++){ // j is column
    //         cout << (int) out_y[i][j]  << " ";
    //     } 
    //     cout << endl;
    // }
    // cout << endl;
}


// Do RLC on the output of the DCT
// Each pair is (# of preceding zeros, value) and the special 0,0 means all the rest of the values in the block are 0
void run_length_coding(vector<int8_t>& out_y, vector<int8_t>& out_cb, vector<int8_t>& out_cr, int8_t tmp_y[8][8], int8_t tmp_cb[8][8], int8_t tmp_cr[8][8]){
    // zig-zag traversal makes it more likely to get many zeros in a row (at higher freqs)
    out_y.push_back(0); out_y.push_back(tmp_y[0][0]);
    out_cb.push_back(0); out_cb.push_back(tmp_cb[0][0]);
    out_cr.push_back(0); out_cr.push_back(tmp_cr[0][0]);
    //cout << (int) tmp_y[0][0] << " ";

    int dir = -1;
    int r = 0; int c = 1;
    int num_zeros[3] = {0, 0, 0};
    for(int d = 1; d < 15; d++){ // d is the diagonal number
        for(int i = 0; i < 8 - abs(d - 7); i++){ // loop through each element in the diagonal
            if (i != 0){
                r -= dir;
                c += dir;
            }
            if (tmp_y[r][c] == 0) num_zeros[0]++;
            else{
                //cout << endl << num_zeros[0] << " " << (int)tmp_y[r][c] << endl;
                out_y.push_back(num_zeros[0]);
                out_y.push_back(tmp_y[r][c]);
                num_zeros[0] = 0;
            } 
            
            if (tmp_cb[r][c] == 0) num_zeros[1]++;
            else {
                out_cb.push_back(num_zeros[1]);
                out_cb.push_back(tmp_cb[r][c]);
                num_zeros[1] = 0;
            }

            if (tmp_cr[r][c] == 0) num_zeros[2]++;
            else{
                out_cr.push_back(num_zeros[2]);
                out_cr.push_back(tmp_cr[r][c]);
                num_zeros[2] = 0;
            }

            //cout << (int) tmp_y[r][c] << " ";
        }
        dir *= -1;
        if (r == 0 || r == 7) c++;
        else if (c == 0 || c == 7) r++;
    }
    // cout << endl;

    // for (int i = 0; i < out_y.size(); i+=2){
    //     cout << "(" << (int)out_y[i] << "," << (int)out_y[i+1] <<  ")" << " ";
    // }
    // cout << endl;
}

// Each thread processes one section of the input file
void* process_block(void* args){
    ThreadInfo* info = (ThreadInfo*)args;
    //cout << info->rowStart << "   " << info->rowNum << endl;

    (info->encoded_y).resize(((*data_ptr)[0].size()/8)*(info->rowNum/8), vector<int8_t>(0));
    (info->encoded_cb).resize(((*data_ptr)[0].size()/8)*(info->rowNum/8), vector<int8_t>(0));
    (info->encoded_cr).resize(((*data_ptr)[0].size()/8)*(info->rowNum/8), vector<int8_t>(0));

    // loop through 8x8 blocks and process each
    int i = 0;
    for (int r = info->rowStart; r < info->rowNum + info->rowStart; r+=8){
        for (int c = 0; c < (*data_ptr)[0].size()-7; c+=8){            
            //cout <<"-----------------" << endl;
            uint8_t tmp_y[8][8];
            uint8_t tmp_cb[8][8];
            uint8_t tmp_cr[8][8];
            int8_t out_y[8][8];
            int8_t out_cb[8][8];
            int8_t out_cr[8][8];

            int y = 0; int cb = 0; int cr = 0;

            for (int i = 0; i < 8; i++){
                for(int j = 0; j < 8; j++){
                    // convert RGB pixels to Y/Cb/Cr
                    y = 16 + (65.738/256)*((int)(*data_ptr)[r+i][c+j][0]) + (129.057/256)*(*data_ptr)[r+i][c+j][1] + (25.064/256)*(*data_ptr)[r+i][c+j][2];
                    cb = 128 - (37.945/256)*(*data_ptr)[r+i][c+j][0] - (74.494/256)*(*data_ptr)[r+i][c+j][1] + (112.439/256)*(*data_ptr)[r+i][c+j][2];
                    cr = 128 + (112.439/256)*(*data_ptr)[r+i][c+j][0] - (94.154/256)*(*data_ptr)[r+i][c+j][1] - (18.285/256)*(*data_ptr)[r+i][c+j][2];
                    tmp_y[i][j] = y;
                    tmp_cb[i][j] = cb;
                    tmp_cr[i][j] = cr;
                }
            }

            if (!use_simd){
                // do the DCT & subsequent quantization on this block
                basic_DCT(tmp_y, tmp_cb, tmp_cr, out_y, out_cb, out_cr);
            }
            else{
                SIMD_DCT(tmp_y, tmp_cb, tmp_cr, out_y, out_cb, out_cr);
            }
  
            // after DCT there should be more zeros/repeated values and the result should be a larger compression with the encoding
            run_length_coding((info->encoded_y)[i], (info->encoded_cr)[i], (info->encoded_cb)[i], out_y, out_cb, out_cr);
            i++;
        }
    }

    return NULL;
}

// lossy compression (JPEG)
void lossy_compression(vector<vector<vector<uint8_t>>>& data, int num_threads, vector<vector<int8_t>>& encoded_data){
    cout << data.size() << " x " << data[0].size() << endl;
    if (use_simd){
        cout << "Using SIMD instructions" << endl;
    }
    data_ptr = &data;

    pthread_mutex_init(&mutexLossyComp, NULL);
    vector<pthread_t*> threads;
    vector<ThreadInfo*> thread_infos;

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
        thread_infos.push_back(info);
        threads.push_back(thread);
    }
    // the last thread gets all the remaining rows
    pthread_t* thread = new pthread_t;
    ThreadInfo* info = new ThreadInfo();
    info->rowStart = rows_per_thread*(num_threads-1);
    info->rowNum = data.size() - info->rowStart;
    pthread_create(thread, NULL, process_block, info);  
    thread_infos.push_back(info);
    threads.push_back(thread);

    // wait for threads to finish
    for (unsigned int x = 0; x < threads.size(); x++){
        pthread_join(*threads[x], NULL);
    }

    // combine encoded data
    // encoded_data[0] is y channel, encoded_data[1] is cb, encoded_data[2] is cr
    for (int i = 0; i < thread_infos.size(); i++) {
        for (int j = 0; j < (thread_infos[i])->encoded_y.size(); j++){
            encoded_data[0].insert( encoded_data[0].end(), ((thread_infos[i])->encoded_y)[j].begin(), ((thread_infos[i])->encoded_y)[j].end() );
            encoded_data[1].insert( encoded_data[1].end(), ((thread_infos[i])->encoded_cb)[j].begin(), ((thread_infos[i])->encoded_cb)[j].end() );
            encoded_data[2].insert( encoded_data[2].end(), ((thread_infos[i])->encoded_cr)[j].begin(), ((thread_infos[i])->encoded_cr)[j].end() );
        }
    }

    pthread_mutex_destroy(&mutexLossyComp);
}

// main to test lossy compression separate from the rest
int main(int argc, char* argv[]){
    int num_threads = 2;
    string filename = "test_images/landscape1.bmp";
    if (argc >= 3){
        filename = argv[1];
        num_threads = stoi(argv[2]);
        if (argc == 4){
            use_simd = (bool) (std::string(argv[3]) == "simd");
        }
            
    }

    // Read in uncompressed image data (BMP)
    vector<vector<vector<uint8_t>>> data;
    vector<uint8_t> bmp_header;
    read_uncompressed(filename, data, bmp_header);

    // pad the image so that we have 8x8 blocks by repeating rows or columns as necessary
    // (this is what JPEG does, the real image size would be included in the metadata
    // so the zeros would be dropped when decompressing)
    if (data.size() % 8 != 0 || data[0].size() % 8 != 0){
        data.resize(((data.size()+7)/8)*8,vector<vector<uint8_t>>(((data[0].size()+7)/8)*8,vector<uint8_t>(3)));
    }

    auto start = std::chrono::high_resolution_clock::now();
    // Do the lossy compression
    precompute_DCT_constants();
    if (use_simd) precompue_DCT_coeff_matrix();
    vector<vector<int8_t>> encoded_data;
    encoded_data.resize(3, vector<int8_t>(0));
    lossy_compression(data, num_threads, encoded_data);

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
    cout << "Compression complete" << endl;
	cout << "Took " << elapsed.count() << " seconds" << endl;

    int original_data_size = 0;
    for(int i = 0; i < data.size(); i++){
        for(int j = 0; j < data[i].size(); j++){
            original_data_size += data[i][j].size();
        }
    }
    cout << "Original data size (bytes) = " << original_data_size << endl;

    int compressed_data_size = encoded_data[0].size() + encoded_data[1].size() + encoded_data[2].size();
    cout << "Final data size (bytes) = " << compressed_data_size << endl;
    cout << "     Y-channel = " << encoded_data[0].size() << " bytes, Cb-channel = " << encoded_data[1].size() << " bytes. Cr-channel = " << encoded_data[2].size() << endl;
    cout << "Total Compression ratio = " << (float)original_data_size/compressed_data_size << endl;

    decompress_and_save(encoded_data, bmp_header, data[0].size(), data.size(), "decompressed_output.bmp");
}
