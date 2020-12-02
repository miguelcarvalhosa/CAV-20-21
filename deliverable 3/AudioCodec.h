

#ifndef AUDIO_CODEC_H
#define AUDIO_CODEC_H


#include <iostream>
#include <string>
#include <sndfile.hh>
#include <vector>
#include <math.h>
#include "GolombEncoder.h"
#include "GolombDecoder.h"



class AudioCodec {

public:

    typedef enum {
        REDUNDANCY_INDEPENDENT,
        REDUNDANCY_MID_SIDE,
        REDUNDANCY_LEFT_SIDE,
        REDUNDANCY_RIGHT_SIDE
    } audioCodec_ChannelRedundancy;

    typedef enum {
        ESTIMATION_NONE,
        ESTIMATION_ADAPTATIVE
    } audioCodec_parameterEstimationMode;

    AudioCodec();

    virtual ~AudioCodec();

    unsigned int estimateM(std::string inputFile, audioCodec_ChannelRedundancy redundancy);

    void compress(std::string inputFile, std::string compressedFile, unsigned int m, audioCodec_ChannelRedundancy redundancy, audioCodec_parameterEstimationMode estimation, unsigned int estimation_nBlocks);

    void decompress(std::string compressedFile, std::string outputFile);


private:

    std::vector<signed long> calculateR_L (signed long val_x, signed long val_y, audioCodec_ChannelRedundancy redundancy);
    std::vector<long signed> calculateX_Y (signed long val_r, signed long val_l, audioCodec_ChannelRedundancy redundancy);
    unsigned int estimateM_fromBlock(unsigned int sum, unsigned int blockSize);

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

};


#endif //AUDIO_CODEC_H
