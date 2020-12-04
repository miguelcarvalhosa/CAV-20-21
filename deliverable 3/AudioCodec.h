/**
 * \brief A class to compress and decompress a .wav audio file.
 *        Includes methods to compress and decompress audio files and to compute histograms and entropies.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */


#ifndef AUDIO_CODEC_H
#define AUDIO_CODEC_H

#include <iostream>
#include <string>
#include <sndfile.hh>
#include <vector>
#include <math.h>
#include <numeric>
#include "GolombEncoder.h"
#include "GolombDecoder.h"


class AudioCodec {

public:

    /**
     * Enumeration with the types of inter-channel redundancy to use when compressing the audio file.
     */
    typedef enum {
        REDUNDANCY_INDEPENDENT,     /**< No redundancy between channels */
        REDUNDANCY_MID_SIDE,        /**< Compress the mean (mid) and the difference (side) of the channels */
        REDUNDANCY_LEFT_SIDE,       /**< Compress the left channel (left) and the difference (side) of the channels */
        REDUNDANCY_RIGHT_SIDE       /**< Compress the right channel (right) and the difference (side) of the channels */
    } audioCodec_ChannelRedundancy;

    /**
     * Enumeration with the modes of the Golomb encoder parameter estimation.
     */
    typedef enum {
        ESTIMATION_NONE,            /**< No estimation of the Golomb encoder parameter */
        ESTIMATION_ADAPTATIVE       /**< Adaptative estimation of the Golomb encoder parameter */
    } audioCodec_parameterEstimationMode;

    /**
     * Enumeration with the modes of losses during encoding.
     */
    typedef enum {
        MODE_LOSSLESS,              /**< Mode with no losses */
        MODE_LOSSY                  /**< Mode with losses */
    } audioCodec_lossMode;

    /**
     * Enumeration with the modes of computing histograms histogram.
     */
    typedef enum {
        MODE_NO_HISTOGRAM,          /**< Do not compute histograms */
        MODE_RESIDUAL_HISTOGRAM     /**< Compute residuals histogram */
    } audioCodec_histogramMode;

    /**
     * \brief Default constructor.
     */
    AudioCodec();

    /**
     * \brief Destructor.
     */
    virtual ~AudioCodec();

    /**
     *  \brief A function to estimate the best Golomb encoder parameter 'm' value for the entire audio file
     *
     *  \param[in] inputFile    A string with the input audio file path
     *  \param[in] redundancy   The type of redundancy to use
     *  \param[in] loss         The type of loss mode
     *  \param[in] lostBits     The number of bits to reduce to each residual
     *
     *  \return The best value for parameter 'm'
     */
    unsigned int estimateM(std::string inputFile, audioCodec_ChannelRedundancy redundancy, audioCodec_lossMode loss, unsigned int lostBits);

    /**
     *  \brief A function to compress an audio file
     *
     *  \param[in] inputFile            A string with the input audio file path
     *  \param[out] compressedFile      A string with the output compressed file path
     *  \param[in] m                    The Golomb encoder 'm' parameter
     *  \param[in] redundancy           The type of redundancy to use
     *  \param[in] estimation           The type of estimation of the value 'm' to use
     *  \param[in] estimation_nBlocks   The number of audio sample blocks to use in each parameter 'm' estimation
     *  \param[in] loss                 The type of loss mode
     *  \param[in] lostBits             The number of bits to reduce to each residual
     *  \param[in] histMode             The type of histogram mode
     *  \param[out] histogramFileName   A string with the output histogram file path
     */
    void compress(std::string inputFile, std::string compressedFile, unsigned int m, audioCodec_ChannelRedundancy redundancy, audioCodec_parameterEstimationMode estimation, unsigned int estimation_nBlocks, audioCodec_lossMode loss, unsigned int lostBits, audioCodec_histogramMode histMode, std::string histogramFileName);

    /**
     *  \brief A function to decompress an audio file
     *
     *  \param[in] compressedFile       A string with the input compressed file path
     *  \param[out] outputFile          A string with the output audio file path
     */
    void decompress(std::string compressedFile, std::string outputFile);

    /**
     *  \brief A function to calculate the histogram of an audio file
     *
     *  \param[out] histFileName         A string with the output histogram file path
     *  \param[in] audioFileName         A string with the input audio file path
     */
    void calculateAudioHist(std::string histFileName, std::string audioFileName);

    /**
     *  \brief  A function to calculate the entropy of an audio file
     *
     *  \param[in] fileName             A string with the input audio file path
     *
     *  \return A float vector with the audio entropy. The vector has three positions: right, left and mono channel entropy.
     */
    std::vector<float> audioEntropy(std::string fileName);

    /**
     *  \brief  A function to calculate the entropy of a compressed file
     *
     *  \param[in] fileName             A string with the input compressed file path
     *
     *  \return A float vector with the audio entropy. The vector has two positions: first and second channel entropy.
     */
    std::vector<float> residualsEntropy(std::string fileName);


private:

    /**
     *  \brief A function to calculate the X and Y channels from the right and left channels
     *
     *  \param[in] val_r                The value of the right channel sample
     *  \param[in] val_l                The value of the left channel sample
     *  \param[in] redundancy           The type of redundancy to use
     *
     *  \return A long vector with the X and Y channel values. The vector has two positions: X and Y channel sample value.
     */
    std::vector<long signed> calculateX_Y (signed long val_r, signed long val_l, audioCodec_ChannelRedundancy redundancy);

    /**
     *  \brief A function to calculate the right and left channels from the X and Y channels
     *
     *  \param[in] val_x                The value of the X channel sample
     *  \param[in] val_y                The value of the Y channel sample
     *  \param[in] redundancy           The type of redundancy to use
     *
     *  \return A long vector with the right and left channel values. The vector has two positions: right and left channel sample value.
     */
    std::vector<signed long> calculateR_L (signed long val_x, signed long val_y, audioCodec_ChannelRedundancy redundancy);

    /**
     *  \param A function to estimate the best Golomb encoder parameter 'm' value for a block of the audio file
     *
     *  \param[in] sum                  The sum of the samples values in the block
     *  \param[in] blockSize            The number of samples in the block
     *
     *  \return The best value for parameter 'm'
     */
    unsigned int estimateM_fromBlock(unsigned int sum, unsigned int blockSize);

    /**
     *  \brief A function to calculate the histogram of a compressed file
     *
     *  \param[out] fileName            A string with the output histogram file path
     *  \param[in] res_x                The value of the X channel residual
     *  \param[in] res_y                The value of the Y channel residual
     *  \param[in] totalFrames          The number of frames in the audio file
     */
    void calculateResHist(std::string fileName, short res_x, short res_y, unsigned int totalFrames);


    static constexpr size_t FRAMES_BUFFER_SIZE = 65536;                     // Size of a frame buffer, in frames
    int nFrames = 0;                                                        // Number of frames in the input audio file
    int nChannels = 0;                                                      // Number of channels in the input audio file
    int sampleRate = 0;                                                     // Sample rate of the input audio file
    int format = 0;                                                         // Format of the input audio file
    unsigned int lastFrameSize = 0;                                         // Number of frames in the last frame buffer read
    unsigned int initial_m = 0;                                             // Initial m parameter used
    unsigned int m = 0;                                                     // Current m parameter used
    audioCodec_ChannelRedundancy redundancy = REDUNDANCY_INDEPENDENT;       // Redundancy mode
    audioCodec_parameterEstimationMode estimation = ESTIMATION_NONE;        // Estimation mode
    unsigned int estimation_nBlocks = 0;                                    // Number of blocks to use in estimation mode
    audioCodec_lossMode loss = MODE_LOSSLESS;                               // Loss mode
    unsigned int lostBits = 0;                                              // Number of bits to reduce in loss mode

};

#endif //AUDIO_CODEC_H
