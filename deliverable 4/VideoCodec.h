

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

    /**
     * Enumeration with the modes of the Golomb encoder parameter estimation.
     */
    typedef enum {
        ESTIMATION_NONE,            /**< No estimation of the Golomb encoder parameter */
        ESTIMATION_ADAPTATIVE       /**< Adaptative estimation of the Golomb encoder parameter */
    } parameterEstimationMode;

    /**
     * Enumeration with the modes of losses during encoding.
     */
    typedef enum {
        MODE_LOSSLESS,              /**< Mode with no losses */
        MODE_LOSSY                  /**< Mode with losses */
    } lossMode;

    VideoCodec(predictorType predictor, unsigned int initial_m, parameterEstimationMode estimation, unsigned int estimation_block_size, lossMode loss, unsigned int lostBits);

    virtual ~VideoCodec();

    void compress(std::string inputFile, std::string compressedFile);
    void decompress(std::string outputFile, std::string compressedFile);

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

    /* Video Coded configurations */
    predictorType predictor = PREDICTOR_LINEAR_JPEG_7;
    parameterEstimationMode estimation = ESTIMATION_NONE;        // M parameter estimation mode
    lossMode loss = MODE_LOSSLESS;                               // codec loss mode
    unsigned int estimation_block_size;
    unsigned int lostBits;
    unsigned int initial_m;

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

    unsigned int estimateM_fromBlock(unsigned int sum, unsigned int blockSize);
};


#endif //VIDEOCODEC_H
