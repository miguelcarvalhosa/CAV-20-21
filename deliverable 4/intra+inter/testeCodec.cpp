
/**
 * \file testeCodec.cpp
 *
 * \brief Contains the resolution of point 18 of the Deliverable 4.
 *        This program creates an instance of the codec, compresses a video file and then decompresses the file.
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

    string inFile = "444_.y4m";                     // Input video file
    string outFile = "restored_video.y4m";          // Output video file
    string cmpFile = "video.cmp";                   // Output compressed file


    unsigned int estimationBlockSize = 1000, intraFramePeriodicity=10;
    unsigned int blockSize = 4, searchArea = 16;

    // Codec instance
    VideoCodec my_codec;

    /* configure codec settings for both intra and inter mode */
    my_codec.setIntraCodingParameters(VideoCodec::PREDICTOR_LINEAR_JPEG_1, intraFramePeriodicity, estimationBlockSize);
    my_codec.setInterCodingParameters(VideoCodec::INTERSPERSED, blockSize, searchArea);

    //my_codec.compress(inFile, cmpFile,10,VideoCodec::ESTIMATION_ADAPTATIVE);
    my_codec.decompress(outFile, cmpFile);


    // Print the results
    cout << "\nInput file '" << inFile << "' size (bytes) :" << getFileSize(inFile) << endl;

    cout << "Compressed file '" << cmpFile << "' size (bytes) :" << getFileSize(cmpFile) << endl;

    cout << "Decompressed file '" << outFile << "' size (bytes) :" << getFileSize(outFile) << endl;

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