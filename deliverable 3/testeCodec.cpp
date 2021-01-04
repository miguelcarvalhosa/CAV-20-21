
/**
 * \file testeCodec.cpp
 *
 * \brief Contains the resolution of the Deliverable 3.
 *        This program creates an instance of the codec, compresses an audio file and then decompresses the file.
 *        The program also computes the files histograms, entropies and the compression ratio.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */

#include "AudioCodec.h"
#include <string>
#include <fstream>

using namespace std;

/*
 * Function prototypes
 */
long getFileSize(string fileName);


int main(int argc, char *argv[]) {

    string inFile = "sample01.wav";         // Input audio file
    string cmpFile = "sample01.cmp";        // Output compressed file
    string decFile = "sample01_out.wav";    // Output audio file

    AudioCodec my_codec;                    // Codec instance

    // Estimate the best 'm' parameter for the audio file
    unsigned int m = my_codec.estimateM(inFile, AudioCodec::REDUNDANCY_MID_SIDE, AudioCodec::MODE_LOSSY,2);
    std::cout << "initial_m: " << m << std::endl;

    // Compress the audio file
    my_codec.compress(inFile, cmpFile, m, AudioCodec::REDUNDANCY_MID_SIDE, AudioCodec::ESTIMATION_ADAPTATIVE, 200, AudioCodec::MODE_LOSSY, 2, AudioCodec::MODE_NO_HISTOGRAM, "residualHistogram_lossy.txt");

    // Decompress the audio file
    my_codec.decompress(cmpFile, decFile);

    // Calculate the histograms of the audio file and the compressed file
    my_codec.calculateAudioHist("inFileHistogram.txt", inFile);
    my_codec.calculateAudioHist("outFileHistogram.txt", decFile);

    // Calculate the entropy of the input, output and compressed files
    vector<float> inFileEntropy = my_codec.audioEntropy("inFileHistogram.txt");
    vector<float> decFileEntropy = my_codec.audioEntropy("outFileHistogram.txt");
    vector<float> cmpFileEntropy = my_codec.residualsEntropy("residualHistogram_lossless.txt");
    // residualHistogram_lossy

    // Print the results
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