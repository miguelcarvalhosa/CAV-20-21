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

    unsigned int estimateM(std::string inputFile, audioCodec_ChannelRedundancy redundancy, audioCodec_lossMode loss, unsigned int lostBits);

    void compress(std::string inputFile, std::string compressedFile, unsigned int m, audioCodec_ChannelRedundancy redundancy, audioCodec_parameterEstimationMode estimation, unsigned int estimation_nBlocks, audioCodec_lossMode loss, unsigned int lostBits, audioCodec_histogramMode histMode, std::string histogramFileName);

    void decompress(std::string compressedFile, std::string outputFile);

    void calculateAudioHist(std::string histFileName, std::string audioFileName);

    std::vector<float> audioEntropy(std::string fileName);

    std::vector<float> residualsEntropy(std::string fileName);


private:

    std::vector<signed long> calculateR_L (signed long val_x, signed long val_y, audioCodec_ChannelRedundancy redundancy);
    std::vector<long signed> calculateX_Y (signed long val_r, signed long val_l, audioCodec_ChannelRedundancy redundancy);
    unsigned int estimateM_fromBlock(unsigned int sum, unsigned int blockSize);
    void calculateResHist(std::string fileName, short res_x, short res_y, unsigned int totalFrames);

    static constexpr size_t FRAMES_BUFFER_SIZE = 65536;
    int nFrames = 0;
    int nChannels = 0;
    int sampleRate = 0;
    int format = 0;
    unsigned int lastFrameSize = 0;
    unsigned int initial_m = 0;
    unsigned int m = 0;
    audioCodec_ChannelRedundancy redundancy = REDUNDANCY_INDEPENDENT;
    audioCodec_parameterEstimationMode estimation = ESTIMATION_NONE;
    unsigned int estimation_nBlocks = 0;
    audioCodec_lossMode loss = MODE_LOSSLESS;
    unsigned int lostBits = 0;

};


#endif //AUDIO_CODEC_H
