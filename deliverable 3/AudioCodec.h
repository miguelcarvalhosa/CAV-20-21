

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
    AudioCodec();

    virtual ~AudioCodec();

    void compress(std::string inputFile, std::string compressedFile, unsigned int m);

    void decompress(std::string compressedFile, std::string outputFile);




private:
    static constexpr size_t FRAMES_BUFFER_SIZE = 65536;
    int nFrames = 0;
    int nChannels = 0;
    int sampleRate = 0;
    int format = 0;
    unsigned int lastFrameSize = 0;
    unsigned int m = 0;

};


#endif //AUDIO_CODEC_H
