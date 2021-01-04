

#ifndef VIDEOCODEC_H
#define VIDEOCODEC_H


#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <fstream>
#include <numeric>
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


    /* delete later, files are used for debug purposes */
    std::ofstream file;
    std::ofstream file2;
    std::ofstream decodedMotion;
    std::ofstream encodedMotion;
    std::ofstream decodedRefBlockPos;
    std::ofstream encodedRefBlockPos;
    std::ofstream decodedBlockFrameValue;
    std::ofstream originalBlockFrameValue;

    std::ofstream decodedBlockPos;
    std::ofstream encodedBlockPos;
    std::ofstream originalFrame;
    std::ofstream decodedFrame;


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

    typedef enum {
        Y,              /**< Mode with no losses */
        U,
        V,
        YUV               /**< Mode with losses */
    } planeComponent;

    class Point {
    public:
        int x, y;
        void setPos(int x, int y) {
            this->x = x;
            this->y = y;
        }
        void setX(int x) {
            this->x = x;
        }
        void setY(int y) {
            this->y = y;
        }
    };

    class blockData {
    public:
        int size;
        int x, y;

        unsigned char* blockBuf_y;
        unsigned char* blockBuf_u;
        unsigned char* blockBuf_v;
        /* unsigned char* frameBlock_y;
        unsigned char* frameBlock_u;
        unsigned char* frameBlock_v; */

        void setSize(int size) {
            this->size = size;
            blockBuf_y =  new unsigned char[size*size];
            blockBuf_u = new unsigned char[size*size];
            blockBuf_v = new unsigned char[size*size];
        }
        void setBlockPos(int x, int y) {
            this->x = x;
            this->y = y;
        }
    };
    class blockEstimationData {
        public:
            //blockData block;
            int size;
            Point motionVector_y, motionVector_u, motionVector_v;
            std::vector<int> residuals_y, residuals_u, residuals_v;

            /* delete later, these attributes are used for debug purposes */
            Point ref_y, ref_u, ref_v;
            unsigned char* blockBuf_y;
            unsigned char* blockBuf_u;
            unsigned char* blockBuf_v;
            unsigned char* refBlockBuf_y;
            unsigned char* refBlockBuf_u;
            unsigned char* refBlockBuf_v;

            void setSize(int size) {
                this->size = size;
            }
            void setMotionVectors(Point motionVector_y, Point motionVector_u, Point motionVector_v ) {
                this->motionVector_y = motionVector_y;
                this->motionVector_u = motionVector_u;
                this->motionVector_v = motionVector_v;
            }
            void setResiduals(std::vector<int> residuals_y, std::vector<int> residuals_u, std::vector<int> residuals_v) {
                this->residuals_y = residuals_y;
                this->residuals_u = residuals_u;
                this->residuals_v = residuals_v;
            }
            void setMotionVector(Point motionVector, planeComponent plane) {
                if(plane == Y) {
                    motionVector_y = motionVector;
                } else if(plane == U) {
                    motionVector_u = motionVector;
                } else {
                    motionVector_v = motionVector;
                }
            }

            /* delete later, these attributes are used for debug purposes */
            void setReferenceBuf(unsigned char* refBlockBuf, planeComponent plane) {
                if(plane == Y) {
                    refBlockBuf_y = refBlockBuf;
                } else if(plane == U) {
                    refBlockBuf_u = refBlockBuf;
                } else {
                    refBlockBuf_v = refBlockBuf;
                }
            }
            /* delete later, these attributes are used for debug purposes */
            void setBlockBuf(unsigned char* blockBuf, planeComponent plane) {
                if(plane == Y) {
                    blockBuf_y = blockBuf;
                } else if(plane == U) {
                    blockBuf_u = blockBuf;
                } else {
                    blockBuf_v = blockBuf;
                }
            }
    };

    class blockLimits {
    public:
        Point topLeftPos, topRightPos, bottomLeftPos;
        void setBlockLimits(Point topLeftPos, Point topRightPos, Point bottomLeftPos) {
            this->topLeftPos = topLeftPos;
            this->topRightPos = topRightPos;
            this->bottomLeftPos = bottomLeftPos;
        }
    };

    typedef enum {
        MODE_INTRA,              /**< Mode with no losses */
        MODE_INTER                  /**< Mode with losses */
    } frameEncodingMode;

    typedef enum {
        EXHAUSTIVE,             /**< Mode with no losses */
        INTERSPERSED,
    } blockSearchMode;


    fileData inFileData;
    fileData outFileData;
    unsigned char* lastFrameBuf;

    /* Video Coded configurations */
    predictorType predictor = PREDICTOR_LINEAR_JPEG_7;
    parameterEstimationMode estimation = ESTIMATION_NONE;        // M parameter estimation mode
    lossMode loss = MODE_LOSSLESS;                               // codec loss mode
    unsigned int estimation_block_size;
    unsigned int lostBits;
    unsigned int initial_m;
    frameEncodingMode encodingMode;



    void encodeIntra(GolombEncoder& encoder, unsigned char* frameBuf);
    void encodeInter(GolombEncoder& encoder, unsigned char* frameBuf, int blockSize, int searchArea);

    unsigned char* decodeIntra(GolombDecoder& decoder);
    unsigned char* decodeInter(GolombDecoder& decoder, int blockSize, int searchArea);

    void encodeFrameBlock(GolombEncoder& encoder, blockEstimationData bestMatchData, planeComponent plane);

    //unsigned char* decodeFrameBlock(GolombDecoder& decoder, int blockSize, int x, int y);
    blockEstimationData decodeFrameBlock(GolombDecoder& decoder, int blockSize, int x, int y);

    blockData getFrameBlock(unsigned char* frameBuf, int x, int y, int blocksize, planeComponent plane);

    blockEstimationData motionEstimation(blockData& currentFrameBlock, int searchArea, planeComponent plane, blockSearchMode searchMode);

    blockLimits calculateBlockLimits(blockData currentFrameBlock, int searchArea);
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
