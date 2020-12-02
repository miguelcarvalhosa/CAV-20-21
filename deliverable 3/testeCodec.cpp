#include "AudioCodec.h"
#include <string>
#include <fstream>

using namespace std;


long getFileSize(string fileName);


int main(int argc, char *argv[]) {

    string inFile = "sample01.wav";
    string cmpFile = "sample01.cmp";
    string decFile = "sample01_out.wav";

    AudioCodec my_codec;
    int m = my_codec.estimateM(inFile, AudioCodec::REDUNDANCY_INDEPENDENT);
    my_codec.compress(inFile, cmpFile, 500, AudioCodec::REDUNDANCY_INDEPENDENT, AudioCodec::ESTIMATION_NONE, 10, AudioCodec::LOSS_LOSSY,13);
    my_codec.decompress(cmpFile, decFile);

    cout << "Input file '" << inFile << "' size (bytes) :" << getFileSize(inFile) << endl;
    cout << "Compressed file '" << cmpFile << "' size (bytes) :" << getFileSize(cmpFile) << endl;
    cout << "Decompressed file '" << decFile << "' size (bytes) :" << getFileSize(decFile) << endl;
    cout << "Compression Ratio: " << (float)getFileSize(inFile)/(float)getFileSize(cmpFile) << endl;

    return 0;
}


/*
 * Returns the file size in bytes
 */
long getFileSize(string fileName) {
    ifstream in_file(fileName, ios::binary);
    in_file.seekg(0, ios::end);
    long file_size = in_file.tellg();
    return file_size;
}