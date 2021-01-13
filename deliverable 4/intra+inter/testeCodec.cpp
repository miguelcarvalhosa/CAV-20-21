
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

#include "VideoCodec.h"
#include <string>
#include <fstream>

using namespace std;

/*
 * Function prototypes
 */
long getFileSize(string fileName);


int main() {

    string inFile = "444_.y4m";         // Input audio file
    string outFile = "restored_video.y4m";         // Input audio file
    string cmpFile = "video.cmp";        // Output compressed file


    unsigned int estimationBlockSize = 1000, lostBits = 0, intraFramePeriodicity=10;
    unsigned int blockSize = 4, searchArea = 16, lostbitsY = 0, lostbitsU = 0, lostbitsV =0;

    // Codec instance
    VideoCodec my_codec(VideoCodec::MODE_LOSSLESS, lostbitsY,lostbitsU,lostbitsV);

    /* configure codec settings for both intra and inter mode */
   // my_codec.setIntraCodingParameters(VideoCodec::PREDICTOR_LINEAR_JPEG_1, intraFramePeriodicity, estimationBlockSize);
    //my_codec.setInterCodingParameters(VideoCodec::INTERSPERSED, blockSize, searchArea);

    //my_codec.compress(inFile, cmpFile,10,VideoCodec::ESTIMATION_ADAPTATIVE);
    my_codec.decompress(outFile, cmpFile);


    // Print the results
//    cout << "\nInput file '" << inFile << "' size (bytes) :" << getFileSize(inFile) << endl;
//    cout << "\t right channel entropy: " << inFileEntropy[0] << endl;
//    cout << "\t left channel entropy: " << inFileEntropy[1] << endl;
//    cout << "\t mono channel entropy: " << inFileEntropy[2] << endl;
//
//    cout << "Compressed file '" << cmpFile << "' size (bytes) :" << getFileSize(cmpFile) << endl;
//    cout << "\t first channel entropy: " << cmpFileEntropy[0] << endl;
//    cout << "\t second channel entropy: " << cmpFileEntropy[1] << endl;
//
//    cout << "Decompressed file '" << decFile << "' size (bytes) :" << getFileSize(decFile) << endl;
//    cout << "\t right channel entropy: " << decFileEntropy[0] << endl;
//    cout << "\t left channel entropy: " << decFileEntropy[1] << endl;
//    cout << "\t mono channel entropy: " << decFileEntropy[2] << endl;
//
//    cout << "Compression Ratio: " << (float)getFileSize(inFile)/(float)getFileSize(cmpFile) << endl;

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