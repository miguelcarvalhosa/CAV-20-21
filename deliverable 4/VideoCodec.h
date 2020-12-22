

#ifndef VIDEOCODEC_H
#define VIDEOCODEC_H


#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <fstream>
#include "GolombEncoder.h"
#include "GolombDecoder.h"


class VideoCodec {

public:

    typedef enum {
        VIDEO_FORMAT_444,
        VIDEO_FORMAT_422,
        VIDEO_FORMAT_420
    } videoFormat;

    typedef enum {
        PREDICTOR_LINEAR_JPEG_1,
        PREDICTOR_LINEAR_JPEG_2,
        PREDICTOR_LINEAR_JPEG_3,
        PREDICTOR_LINEAR_JPEG_4,
        PREDICTOR_LINEAR_JPEG_5,
        PREDICTOR_LINEAR_JPEG_6,
        PREDICTOR_LINEAR_JPEG_7,
        PREDICTOR_NON_LINEAR_JPEGLS
    } predictorType;

    VideoCodec();

    virtual ~VideoCodec();

    void compress(std::string inputFile, std::string compressedFile, predictorType predictor);



private:

    typedef struct {
        std::string header;
        int width;
        int height;
        int uv_width;
        int uv_height;
        double fps;
        videoFormat format;
    } fileData;

    typedef struct {
        std::string inFileHeader;
        int frameCount;
        predictorType predictor;
        int golombM;
    } compressedFileData;

    fileData inFileData;
    fileData outFileData;

    // Returns the next frame in the file, in the specified color format
    // only works with the infiledata structure
    unsigned char* readFrame(std::ifstream* fp, videoFormat format);

    void writeFrame(std::ofstream* fp, unsigned char* frameBuf);

    unsigned char* convertFrame_422to444(unsigned char* frameBuf);

    unsigned char* convertFrame_420to444(unsigned char* frameBuf);

    unsigned char* convertFrame_444to420(unsigned char* frameBuf);

    // Returns a structure containing the data extracted from the header of the file
    fileData parseHeader(std::string header);

    int predict(int left_sample, int top_sample, int top_left_sample, predictorType predictor);

};


#endif //VIDEOCODEC_H
