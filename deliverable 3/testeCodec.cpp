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
    unsigned int m = my_codec.estimateM(inFile, AudioCodec::REDUNDANCY_INDEPENDENT, AudioCodec::MODE_LOSSLESS,1);
    std::cout << "initial_m: " << m << std::endl;
    my_codec.compress(inFile, cmpFile, 500, AudioCodec::REDUNDANCY_INDEPENDENT, AudioCodec::ESTIMATION_ADAPTATIVE, 10, AudioCodec::MODE_LOSSLESS, 1, AudioCodec::MODE_RESIDUAL_HISTOGRAM, "residualHistogram.txt");
    my_codec.decompress(cmpFile, decFile);

    my_codec.calculateAudioHist("inFileHistogram.txt", inFile);
    my_codec.calculateAudioHist("outFileHistogram.txt", decFile);

    vector<float> inFileEntropy = my_codec.audioEntropy("inFileHistogram.txt");
    vector<float> decFileEntropy = my_codec.audioEntropy("outFileHistogram.txt");
    vector<float> cmpFileEntropy = my_codec.residualsEntropy("residualHistogram.txt");

    cout << "\nInput file '" << inFile << "' size (bytes) :" << getFileSize(inFile) << endl;
    cout << "\t right channel entropy: " << inFileEntropy[0] << endl;
    cout << "\t left channel entropy: " << inFileEntropy[1] << endl;
    cout << "\t mono channel entropy: " << inFileEntropy[2] << endl;

    cout << "Compressed file '" << cmpFile << "' size (bytes) :" << getFileSize(cmpFile) << endl;
    cout << "\t first channel entropy: " << cmpFileEntropy[0] << endl;
    cout << "\t second channel entropy: " << cmpFileEntropy[1] << endl;

    cout << "Decompressed file '" << decFile << "' size (bytes) :" << getFileSize(decFile) << endl;
    cout << "\t right channel entropy: " << decFileEntropy[0] << endl;
    cout << "\t left channel entropy: " << decFileEntropy[1] << endl;
    cout << "\t mono channel entropy: " << decFileEntropy[2] << endl;

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