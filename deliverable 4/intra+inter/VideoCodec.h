/**
 * \brief A class to compress and decompress a .y4m video file.
 *        This codec works with YUV4MPEG2 uncompressed video files in three different color formats: YUV444, YUV422 and
 *        YUV420. The output of the decompressor is always in YUV420 color format.
 *        This lossless version of the codec includes an hybrid intra and inter-frame encoder using predictive coding
 *        and motion compensation.
 *        The user can choose the inter-frame block size and search area, as well as the periodicity of the intra-frame
 *        encoding.
 *        The user can also choose the type of predictor among the seven linear JPEG predictors or the JPEG-LS
 *        non-linear predictor.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */

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

    /**
     * Enumeration with the types of video color format.
     */
    typedef enum {
        VIDEO_FORMAT_444,       // YUV444
        VIDEO_FORMAT_422,       // YUV422
        VIDEO_FORMAT_420        // YUV420
    } videoFormat;

    /**
     * Enumeration with the types of predictors (seven linear JPEG and one non-linear JPEG-LS).
     */
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
     * Enumeration with the search modes that can be used during motion estimation.
     */
    typedef enum {
        EXHAUSTIVE,             /**< Mode where all possible blocks within the search area are tested */
        INTERSPERSED,
    } blockSearchMode;

    /**
     * \brief Constructor.
     */
    VideoCodec();

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
    *  \param[in]  inputFile           A string with the input video file path
    *  \param[out] compressedFile      A string with the output compressed file path
    *  \param[in]  initialm            Initial value of the Golomb encoder parameter
    *  \param[in]  estimation          Mode of the Golomb encoder parameter estimation
    */
    void compress(std::string &inputFile, std::string &compressedFile, unsigned int initialm, parameterEstimationMode estimation);

    /**
    *  \brief A function to decompress a video file
    *
    *  \param[in] outputFile           A string with the output video file path
    *  \param[out] compressedFile      A string with the output compressed file path
    */
    void decompress(std::string &outputFile, std::string &compressedFile);


private:

    /**
     * Structure with some file data parameters.
     */
    typedef struct {
        std::string header;                     // File header
        int width;                              // Frame Y component width in pixels
        int height;                             // Frame Y component height in pixels
        int uv_width;                           // Frame U and V component width in pixels
        int uv_height;                          // Frame U and V component height in pixels
        double fps;                             // Video frames per second
        videoFormat format;                     // Video color format
        int golombM;                            // Golomb encoder m parameter
        int frameCount;                         // Number of frames in the video
        unsigned int estimationBlockSize;       // Size of the block used to estimate the m parameter during intra coding
        unsigned int blockSize;                 // Width and height of each frame block used during inter coding
        unsigned int intraFramePeriodicity;     // Periodicity of the intra frame coding
        unsigned int searchArea;                // Area investigated around the previous frame block during inter coding

        blockSearchMode searchMode;             // Type of search used to find the best block match within the search area during inter coding
        parameterEstimationMode estimation;     // Mode of the Golomb encoder parameter estimation
        predictorType predictor;                // Spatial predictor type used during intra coding
    } fileData;

    /**
    * Enumeration with the different image planes combinations.
    */
    typedef enum {
        Y,              /**< Mode with no losses */
        U,
        V,
        YUV               /**< Mode with losses */
    } planeComponent;

    fileData inFileData;
    predictorType predictor = PREDICTOR_LINEAR_JPEG_7;
    parameterEstimationMode estimation = ESTIMATION_NONE;
    unsigned int estimationBlockSize;
    unsigned int intraFramePeriodicity;
    unsigned int initial_m;
    unsigned int blockSize;
    unsigned int searchArea;
    blockSearchMode searchMode;


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
    void encodeInter(GolombEncoder& encoder, unsigned char* &frameBuf, unsigned char* &lastFrameBuf, int blockSize, int searchArea);

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
    void encodeFrameBlock(GolombEncoder& encoder, blockEstimationData& bestMatchData, planeComponent plane);

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
    unsigned char* decodeFrameBlock(GolombDecoder& decoder, unsigned char* &lastFrameBuf, int blockSize, int x, int y);

    /**
    *  \brief A function to fetch the YUV blocks with x,y coordinates from a frame
    *
    *  \param[in] frameBuf      Pointer to the data buffer of the frame
    *  \param[in] x             x coordinate of the block
    *  \param[in] y             y coordinate of the block
    *  \param[in] blockSize     Size of the block
    *  \return Pointer to the data buffer containing the YUV blocks
    */
    unsigned char* getFrameBlock(unsigned char* &frameBuf, int x, int y, int blockSize);

    /**
    *  \brief A function to fetch a block with x,y coordinates from a frame for a particular image plane
    *
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
    *
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
    *
    *  \param[in] x             x coordinate of the block
    *  \param[in] y             y coordinate of the block
    *  \param[in] blockSize     Size of the block
    *  \param[in] searchArea    Area surrounding the block
    *  \return Object of type blockLimits that contains the coordinates of the top left, top right and
    *  bottom left pixels of the search block
    */
    blockLimits calculateBlockLimits(int x, int y, int blockSize, int searchArea);

    /**
     *  \brief     A function to read the next frame in the file
     *
     *  \param[in] fp            Input file stream
     *  \return    A pointer to the frame
     */
    unsigned char* readFrame(std::ifstream* fp);

    /**
     *  \brief     A function to write a frame to the file
     *
     *  \param[in]  fp              Output file stream
     *  \param[out] frameBuf        A pointer to the frame
     */
    void writeFrame(std::ofstream* fp, unsigned char* frameBuf);

    /**
     *  \brief     A function to convert a frame in color format 422 to 444
     *
     *  \param[in] frameBuf         A pointer to the input frame
     *  \return    A pointer to the output frame
     */
    unsigned char* convertFrame_422to444(unsigned char* frameBuf);

    /**
     *  \brief     A function to convert a frame in color format 420 to 444
     *
     *  \param[in] frameBuf         A pointer to the input frame
     *  \return    A pointer to the output frame
     */
    unsigned char* convertFrame_420to444(unsigned char* frameBuf);

    /**
     *  \brief     A function to convert a frame in color format 444 to 420
     *
     *  \param[in] frameBuf         A pointer to the input frame
     *  \return    A pointer to the output frame
     */
    unsigned char* convertFrame_444to420(unsigned char* frameBuf);

    /**
     *  \brief     A function to convert a frame in color format 422 to 420
     *
     *  \param[in] frameBuf         A pointer to the input frame
     *  \return    A pointer to the output frame
     */
    unsigned char* convertFrame_422to420(unsigned char* frameBuf);

    /**
     *  \brief     A function to parse the file header and extract the data
     *
     *  \param[in] header           A string with the header
     *  \return    A structure with the file data
     */
    fileData parseHeader(std::string header);

    /**
     *  \brief     A function compute the prediction from a sample using a predictor
     *
     *  \param[in] left_sample          The sample on the left of the sample to be predicted
     *  \param[in] top_sample           The sample on the top of the sample to be predicted
     *  \param[in] top_left_sample      The sample on the top and left of the sample to be predicted
     *  \param[in] predictor            The predictor type to be used
     *  \return    The predicted sample
     */
    int predict(int left_sample, int top_sample, int top_left_sample, predictorType predictor);

    /**
     *  \brief A function to estimate the best Golomb encoder parameter 'm' value for a block of the video file
     *
     *  \param[in] sum                  The sum of the samples values in the block
     *  \param[in] blockSize            The number of samples in the block
     *
     *  \return The best value for parameter 'm'
     */
    unsigned int estimateM_fromBlock(unsigned int sum, unsigned int blockSize);

    void compressedHeaderBuild(std::string& compressedHeader, int m, int nFrames);

    int calcNFrames(std::string inputFile, fileData inFileData);

    int estToInt(parameterEstimationMode estimation);

    fileData parseCompressedHeader(std::string header);

};


#endif //VIDEOCODEC_H
