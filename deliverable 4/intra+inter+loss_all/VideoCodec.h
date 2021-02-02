

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
        /* the 7 JPEG linear predictors */
        PREDICTOR_LINEAR_JPEG_1,
        PREDICTOR_LINEAR_JPEG_2,
        PREDICTOR_LINEAR_JPEG_3,
        PREDICTOR_LINEAR_JPEG_4,
        PREDICTOR_LINEAR_JPEG_5,
        PREDICTOR_LINEAR_JPEG_6,
        PREDICTOR_LINEAR_JPEG_7,
        /* non-linear predictor of JPEG-LS */
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

    /**
     * Enumeration with the search modes that can be used during
     * motion estimation
     */
    typedef enum {
        EXHAUSTIVE,             /**< Mode where all possible blocks within the search area are tested */
        INTERSPERSED,
    } blockSearchMode;

    /**
     * \brief Constructor.
     *
     * \param[in] initial_m    Initial m parameter of the Golomb encoder. If the estimation type
     *                         chosen is NONE this value is kept constant during all encoding process
     * \param[in] estimation   Estimation mode of m parameter of the Golomb encoder
     * \param[in] loss         Loss mode of video coded
     * \param[in] lostBits     Quantization step
     */
    VideoCodec(lossMode loss, unsigned int lostBitsY,unsigned int lostBitsU, unsigned int lostBitsV);
    /**
     * \brief Destructor.
     */
    virtual ~VideoCodec();

    /**
     * \brief A function to set the options of the intra coding mode
     *
     * \param[in] predictor                Spatial predictor type
     * \param[in] intraFramePeriodicity    Periodicity of the intra frames
     * \param[in] estimationBlockSize      Size of the block used to estimate the m parameter during intra coding
     */
    void setIntraCodingParameters(predictorType predictor, unsigned int intraFramePeriodicity, unsigned int estimationBlockSize);
    /**
     * \brief A function to set the options of the inter coding mode
     *
     * \param[in] searchMode      Type of search used to find the best block match within the search area
     * \param[in] blockSize       Width and height of each frame block
     * \param[in] searchArea      Area investigated around the previous frame block
     */
    void setInterCodingParameters(blockSearchMode searchMode, unsigned int blockSize, unsigned int searchArea);

    /**
    *  \brief A function to compress a video file
    *
    *  \param[in] inputFile            A string with the input video file path
    *  \param[out] compressedFile      A string with the output compressed file path
    */
    void compress(std::string &inputFile, std::string &compressedFilen, unsigned int initialm,parameterEstimationMode estimation);
    /**
    *  \brief A function to decompress a video file
    *
    *  \param[in] outputFile           A string with the output video file path
    *  \param[out] compressedFile      A string with the output compressed file path
    */
    void decompress(std::string &outputFile, std::string &compressedFile);

private:


    typedef struct {
        std::string header;
        int width;
        int height;
        int uv_width;
        int uv_height;
        double fps;
        videoFormat format;
        int golombM;
        int frameCount;
        unsigned int estimationBlockSize;
        unsigned int blockSize;
        unsigned int intraFramePeriodicity;
        unsigned int searchArea;

        blockSearchMode searchMode;
        parameterEstimationMode estimation;
        predictorType predictor;
    } fileData;

    /**
    * Enumeration of the different image planes combinations
    */
    typedef enum {
        Y,              /**< Mode with no losses */
        U,
        V,
        YUV               /**< Mode with losses */
    } planeComponent;

    /**
    * Class used to store the (x,y) coordinates of a point or vector
    */
    class Point {
        public:
            /* coordinates */
            int x, y;
            void setPos(int x, int y) {
                this->x = x;
                this->y = y;
            }
    };

    /**
    * Class used to store data about the best block match found in the previous frame: motion vectors
    * for each plane and the corresponding residuals.
    */
    class blockEstimationData {
        public:
            /* frame block size */
            int size;
            /* vectors that store the difference between the position of the current block and
             * the position of the best block match found in the previous frame */
            Point motionVector_y, motionVector_u, motionVector_v;
            /* arrays that store for each plane the difference between the current block values and
             * the best block match values found in the previous frame */
            std::vector<short> residuals_y, residuals_u, residuals_v;

            void setMotionVectors(Point motionVector_y, Point motionVector_u, Point motionVector_v ) {
                this->motionVector_y = motionVector_y;
                this->motionVector_u = motionVector_u;
                this->motionVector_v = motionVector_v;
            }

            void setResiduals(std::vector<short> residuals_y, std::vector<short> residuals_u, std::vector<short> residuals_v) {
                this->residuals_y = residuals_y;
                this->residuals_u = residuals_u;
                this->residuals_v = residuals_v;
            }
            void setResiduals(std::vector<short> residuals, planeComponent plane) {
                if(plane == Y) {
                    this->residuals_y = residuals;
                } else if(plane == U){
                    this->residuals_u = residuals;
                } else if(plane == V) {
                    this->residuals_v = residuals;
                }
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
            void setSize(int size) {
                this->size = size;
            }
    };

    /**
    * Class used to store the limits of the search block around each block.
    */
    class blockLimits {
    public:
        /* variables that store the coordinates of the top left, top right and
         * bottom left pixels of the search block */
        Point topLeftPos, topRightPos, bottomLeftPos;
        void setBlockLimits(Point topLeftPos, Point topRightPos, Point bottomLeftPos) {
            this->topLeftPos = topLeftPos;
            this->topRightPos = topRightPos;
            this->bottomLeftPos = bottomLeftPos;
        }
    };

    typedef enum {
        MODE_INTRA,              /**< Mode that uses spatial prediction */
        MODE_INTER               /**< Mode that uses temporal prediction */
    } frameEncodingMode;


    fileData inFileData;

    /* Video Coded configurations */
    predictorType predictor = PREDICTOR_LINEAR_JPEG_7;
    parameterEstimationMode estimation = ESTIMATION_NONE;        // M parameter estimation mode
    lossMode loss = MODE_LOSSLESS;                               // codec loss mode
    unsigned int estimationBlockSize;                            // Size of the block used to estimate the m parameter during intra coding
    unsigned int lostBits;                                       // Quantization step
    unsigned int intraFramePeriodicity;                          // Indication of the periodicity of the key frames

    unsigned int initial_m;                                      // Initial m parameter of the Golomb encoder
    unsigned int blockSize;                                      // Width and height of each frame block
    unsigned int searchArea;                                     // Area investigated around the previous frame block
    unsigned int lostBitsY;
    unsigned int lostBitsU;
    unsigned int lostBitsV;
    blockSearchMode searchMode;                                  // Type of search used to find the best block match within the search area


    /**
    *  \brief A function to encode a frame in INTRA mode
    *
    *  \param[in] encoder      Golomb encoder object used to write to the compressed video file
    *  \param[in] frameBuf     Pointer to the data buffer of the frame to be encoded
    */
    void encodeIntra(GolombEncoder& encoder, unsigned char* &frameBuf);

    /**
    *  \brief A function to encode a frame in INTER mode
    *
    *  \param[in] encoder        Golomb encoder object used to write to the compressed video file
    *  \param[in] frameBuf       Pointer to the data buffer of the frame to be encoded
    *  \param[in] lastFrameBuf   Pointer to the data buffer of the previous encoded frame
    *  \param[in] blockSize      Size of each frame block
    */
    //void encodeInter(GolombEncoder& encoder, unsigned char* &frameBuf, unsigned char* &lastFrameBuf, int blockSize, int searchArea);
    void encodeInter(GolombEncoder& encoder, unsigned char* &frameBuf, unsigned char* &lastFrameBuf,unsigned char* &decodedFrameBuf, int blockSize, int searchArea);
    /**
    *  \brief A function to decode a frame in INTRA mode
    *
    *  \param[in] decoder       Golomb decoder object used to read from the compressed video file
    *  \return Pointer to the data buffer of the decoded frame
    */
    unsigned char* decodeIntra(GolombDecoder& decoder);

    /**
    *  \brief A function to decode a frame in INTER mode
    *
    *  \param[in] decoder       Golomb decoder object used to read from the compressed video file
    *  \param[in] lastFrameBuf  Pointer to the data buffer of the previous decoded frame
    *  \param[in] blockSize     Size of each frame block
    *  \return Pointer to the data buffer of the decoded frame
    */
    unsigned char* decodeInter(GolombDecoder& decoder, unsigned char* &lastFrameBuf, int blockSize);

    /**
    *  \brief A function to encode a frame block in INTER mode
    *
    *  \param[in] encoder         Golomb encoder object used to write to the compressed video file
    *  \param[in] bestMatchData   Best block match data obtained during motion estimation
    *  \param[in] plane           Indication of the image planes that have information to be encoded
    */
    //void VideoCodec::encodeFrameBlock(GolombEncoder& encoder, blockEstimationData& bestMatchData, unsigned  int x, unsigned int y, unsigned char* &lastFrameBuf, unsigned char* &decodedFrameBuf, planeComponent plane);
    void encodeFrameBlock(GolombEncoder& encoder, blockEstimationData& bestMatchData, unsigned  int x, unsigned int y, unsigned char* &lastFrameBuf, unsigned char* &decodedFrameBuf, planeComponent plane);

    /**
    *  \brief A function to decode a frame block in INTER mode
    *
    *  \param[in] decoder       Golomb decoder object used to read from the compressed video file
    *  \param[in] lastFrameBuf  Pointer to the data buffer of the previous decoded frame
    *  \param[in] blockSize     Size of each frame block
    *  \param[in] x             x coordinate of the block to be decoded
    *  \param[in] y             y coordinate of the block to be decoded
    *  \return Pointer to the data buffer of the decoded frame block
    */
    //unsigned char* decodeFrameBlock(GolombDecoder& decoder, unsigned char* &lastFrameBuf, int blockSize, int x, int y);
    void decodeFrameBlock(GolombDecoder& decoder, unsigned char* &lastFrameBuf, unsigned  char* &frameBlockBuf, int blockSize, int x, int y);
    /**
    *  \brief A function to fetch the YUV blocks with x,y coordinates from a frame
    *  \param[in] frameBuf      Pointer to the data buffer of the frame
    *  \param[in] x             x coordinate of the block
    *  \param[in] y             y coordinate of the block
    *  \param[in] blockSize     Size of the block
    *  \return Pointer to the data buffer containing the YUV blocks
    */
    unsigned char* getFrameBlock(unsigned char* &frameBuf, int x, int y, int blockSize);

    /**
    *  \brief A function to fetch a block with x,y coordinates from a frame for a particular image plane
    *  \param[in] frameBuf      Pointer to the data buffer of the frame
    *  \param[in] x             x coordinate of the block
    *  \param[in] y             y coordinate of the block
    *  \param[in] blockSize     Size of the block
    *  \param[in] plane         Plane from which the block has to be fetched
    *  \return Pointer to the data buffer containing the block in the specified image plane
    */
    unsigned char* getFrameBlock_component(unsigned char* &frameBuf, int x, int y, int blockSize, planeComponent plane);

    /**
    *  \brief A function to perform motion estimation on the current frame block
    *  \param[in] frameBlockBuf   Pointer to the data buffer of the current block frame
    *  \param[in] lastFrameBuf    Pointer to the data buffer of the previous decoded frame
    *  \param[in] block_x         x coordinate of the current block
    *  \param[in] block_y         y coordinate of the current block
    *  \param[in] blockSize       Size of the block
    *  \param[in] searchArea      Area investigated around the previous frame block
    *  \param[in] plane           Indication of the image planes where motion estimation needs to be performed
    *  \param[in] searchMode      Type of search used to find the best block match within the search area
    *  \return Object of type blockEstimationData that contains the motion vectors for each plane and the corresponding residuals.
    */
    blockEstimationData motionEstimation(unsigned char* &frameBlockBuf, unsigned char* &lastFrameBuf, int block_x, int block_y, int blockSize, int searchArea, planeComponent plane, blockSearchMode searchMode);

    /**
    *  \brief A function to calculate the coordinates of the search area around a block
    *  \param[in] x             x coordinate of the block
    *  \param[in] y             y coordinate of the block
    *  \param[in] blockSize     Size of the block
    *  \param[in] searchArea    Area surrounding the block
    *  \return Object of type blockLimits that contains the coordinates of the top left, top right and
    *  bottom left pixels of the search block
    */
    blockLimits calculateBlockLimits(int x, int y, int blockSize, int searchArea);


    // Returns the next frame in the file, in the specified color format
    // only works with the infiledata structure
    unsigned char* readFrame(std::ifstream* fp);

    void writeFrame(std::ofstream* fp, unsigned char* frameBuf);

    unsigned char* convertFrame_422to444(unsigned char* frameBuf);

    unsigned char* convertFrame_420to444(unsigned char* frameBuf);

    unsigned char* convertFrame_444to420(unsigned char* frameBuf);

    unsigned char* convertFrame_422to420(unsigned char* frameBuf);

    // Returns a structure containing the data extracted from the header of the file
    fileData parseHeader(std::string header);

    int predict(int left_sample, int top_sample, int top_left_sample, predictorType predictor);

    unsigned int estimateM_fromBlock(unsigned int sum, unsigned int blockSize);

    void compressedHeaderBuild(std::string& compressedHeader, int m, int nFrames);

    int calcNFrames(std::string inputFile, fileData inFileData);

    int estToInt(parameterEstimationMode estimation);

    fileData parseCompressedHeader(std::string header);

};


#endif //VIDEOCODEC_H
