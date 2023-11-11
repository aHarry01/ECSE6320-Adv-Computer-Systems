#include "encode.h"

uint64_t ColumnHash(string const& word) {
    // According to a test on the column.txt, 
    // one feature for all words are that they are less than 12 characters.
    // So, if we use 5 bits to represent each character, and each 5 bit represents a letter in the hash value,
    // a 64-bit hash value can hold 12 letters, which is enough for each word in the file to garantee a UNIQUE hash value!
    // One other benefit is that in fact we can decode this value and turn it back into a word, very useful
    const int mul = 32;
    int64_t hash_value = 0;
    for (char c : word) {
        hash_value += hash_value * mul + (c-'a'+1);
    }
    return hash_value;
}
uint64_t Hash(string const& word) {
    const int mul = 32;
    int64_t hash_value = 0;
    for (char c : word) {
        hash_value += hash_value * mul + (c-'a'+1);
    }
    return hash_value;
}

int Compress(string ENCODED_FILE_PATH, string COMPRESSED_FILE_PATH)
{
    const char* fileName = ENCODED_FILE_PATH.c_str();
    const char* compressedFileName = COMPRESSED_FILE_PATH.c_str();
    FILE* data = fopen(fileName, "rb");
    FILE* compressedData = fopen(compressedFileName, "w");

    int ret, flush, level = Z_DEFAULT_COMPRESSION;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    do {
        strm.avail_in = fread(in, 1, CHUNK, data);
        if (ferror(data)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(data) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, compressedData) != have || ferror(compressedData)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);

    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);

    (void)deflateEnd(&strm);

    fclose(data);
    fclose(compressedData);
    return Z_OK;
}

vector<uint64_t> Decompress(string compressedDataPath, unsigned int length)
{
    ifstream infile(compressedDataPath, ios::binary);
    infile.seekg(0, std::ios::end);
    std::streampos file_size = infile.tellg();
    infile.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(file_size);
    infile.read(reinterpret_cast<char*>(data.data()), file_size);
    infile.close();

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    inflateInit(&stream);

    stream.next_in = data.data();
    stream.avail_in = data.size();

    std::vector<uint64_t> decompressed_data;
    decompressed_data.resize(length); //Should be enough...

    stream.next_out = reinterpret_cast<Bytef*>(decompressed_data.data());
    stream.avail_out = decompressed_data.size() * sizeof(uint64_t);

    inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    cout << "Decompress finished" << endl;
    return decompressed_data;
}