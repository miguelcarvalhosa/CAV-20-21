
/**
 * \file testeCodec.cpp
 *
 * \brief Contains the resolution of point 19 of the Deliverable 4.
 *        This program creates an instance of the codec, compresses a video file and then decompresses the file.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */

#include "VideoCodec.h"
#include <string>
#include <fstream>
#include <chrono>
#include <ratio>
#include <ctime>
using namespace std;

/*
 * Function prototypes
 */
long getFileSize(string fileName);


int main() {

    string inFile = "444_.y4m";                     // Input video file
    string outFile = "restored_video.y4m";          // Output video file
    string cmpFile = "video.cmp";                   // Output compressed file


    chrono::steady_clock::time_point t1, t2, t3;

    unsigned int lostBitsY = 0, lostBitsU = 0, lostBitsV = 0;
    unsigned int estimationBlockSize = 1000, initial_m = 10;
    unsigned int intraFramePeriodicity = 1, blockSize = 0, searchArea = 0;

    // Codec instance
    VideoCodec my_codec(VideoCodec::MODE_LOSSLESS, lostBitsY, lostBitsU, lostBitsV, VideoCodec::MODE_NO_HISTOGRAM);

    /* configure codec settings for intra and inter modes */
    my_codec.setIntraCodingParameters(VideoCodec::PREDICTOR_LINEAR_JPEG_7, intraFramePeriodicity, estimationBlockSize);
    my_codec.setInterCodingParameters(VideoCodec::EXHAUSTIVE, blockSize, searchArea);

    t1 = chrono::steady_clock::now();
    my_codec.compress(inFile, cmpFile, initial_m, VideoCodec::ESTIMATION_ADAPTATIVE);
    t2 = chrono::steady_clock::now();
    my_codec.decompress(outFile, cmpFile);
    t3 = chrono::steady_clock::now();

    chrono::duration<double> compressTime = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
    chrono::duration<double> decompressTime = chrono::duration_cast<chrono::duration<double>>(t3 - t2);


    // Print the results
    cout << "\n\n\n######################### STATISTICS #########################" << endl;
    cout << "PARAMETERS:" << endl;
    cout << "lostBitsY=" << lostBitsY << ", lostBitsU=" << lostBitsU << ", lostBitsV=" << lostBitsV << endl;
    cout << "estimationBlockSize=" << estimationBlockSize << ", initial_m=" << initial_m << endl;
    cout << "intraFramePeriodicity=" << intraFramePeriodicity << ", blockSize=" << blockSize << ", searchArea=" << searchArea << endl;
    cout << "SIZES AND COMPRESSION RATIO" << endl;
    cout << "Input file '" << inFile << "' size (bytes) :" << getFileSize(inFile) << endl;
    cout << "Compressed file '" << cmpFile << "' size (bytes) :" << getFileSize(cmpFile) << endl;
    cout << "Decompressed file '" << outFile << "' size (bytes) :" << getFileSize(outFile) << endl;
    cout << "Compression Ratio: " << (float)getFileSize(inFile)/(float)getFileSize(cmpFile) << endl;
    cout << "Duration (compression): " << compressTime.count() << " seconds"<< endl;
    cout << "Duration (decompression): " << decompressTime.count() << " seconds"<< endl;

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