/**
 * \brief A class to compress and decompress a .y4m video file.
 *        This codec works with YUV4MPEG2 uncompressed video files in three different color formats: YUV444, YUV422 and
 *        YUV420. The output of the decompressor is always in YUV420 color format.
 *        This lossless version of the codec includes an intra-frame encoder using predictive coding.
 *        The user can choose the type of predictor among the seven linear JPEG predictors or the JPEG-LS non-linear
 *        predictor.
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
     * \param[in] estimationBlockSize      Size of the block used to estimate the m parameter during intra coding
     */
    void setIntraCodingParameters(predictorType predictor, unsigned int estimationBlockSize);

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

        parameterEstimationMode estimation;     // Mode of the Golomb encoder parameter estimation
        predictorType predictor;                // Spatial predictor type used during intra coding
    } fileData;

    fileData inFileData;
    predictorType predictor = PREDICTOR_LINEAR_JPEG_7;
    parameterEstimationMode estimation = ESTIMATION_NONE;
    unsigned int estimationBlockSize;
    unsigned int initial_m;

    /**
    *  \brief A function to encode a frame in INTRA mode
    *
    *  \param[in] encoder      Golomb encoder object used to write to the compressed video file
    *  \param[in] frameBuf     Pointer to the data buffer of the frame to be encoded
    */
    void encodeIntra(GolombEncoder& encoder, unsigned char* &frameBuf);

    /**
    *  \brief A function to decode a frame in INTRA mode
    *
    *  \param[in] decoder       Golomb decoder object used to read from the compressed video file
    *  \return Pointer to the data buffer of the decoded frame
    */
    unsigned char* decodeIntra(GolombDecoder& decoder);

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
