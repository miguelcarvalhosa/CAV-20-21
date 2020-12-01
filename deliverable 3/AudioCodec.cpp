

#include "AudioCodec.h"




AudioCodec::AudioCodec() {

}

AudioCodec::~AudioCodec() {

}

void AudioCodec::compress(std::string inputFile, std::string compressedFile, unsigned int m) {
    SndfileHandle sndFileIn {inputFile};
    if(sndFileIn.error()) {
        std::cerr << "Error: invalid input file" << std::endl;
    }
    if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        std::cerr << "Error: file is not in WAV format" << std::endl;
    }
    if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        std::cerr << "Error: file is not in PCM_16 format" << std::endl;
    }

    nFrames = sndFileIn.frames();
    nChannels = sndFileIn.channels();
    sampleRate = sndFileIn.samplerate();
    format = sndFileIn.format();
    this->m = m;

    GolombEncoder encoder(this->m, compressedFile);


    std::vector<short> samplesIn(FRAMES_BUFFER_SIZE * nChannels);
    int framesRead = 0;
    unsigned char firstFrameFlag = 1;

    short buf_r, buf_l;
    short res0_r, res0_l, prev_res0_r, prev_res0_l;
    short res1_r, res1_l, prev_res1_r, prev_res1_l;
    short res2_r, res2_l, prev_res2_r, prev_res2_l;
    short res3_r, res3_l;

    while (framesRead = sndFileIn.readf(samplesIn.data(), FRAMES_BUFFER_SIZE)) {
        if(framesRead != FRAMES_BUFFER_SIZE) {
            lastFrameSize = framesRead;
        }

        for(unsigned int i=0; i < framesRead*nChannels; i += nChannels) {
            buf_r = samplesIn[i];
            buf_l = samplesIn[i+1];
            if(firstFrameFlag == 1) {
                if(i == 0) {
                    res0_r = buf_r;
                    encoder.encode(res0_r);
                    prev_res0_r = res0_r;
                    res0_l = buf_l;
                    encoder.encode(res0_l);
                    prev_res0_l = res0_l;
                }
                else if( i == 2) {
                    res0_r = buf_r;
                    res1_r = res0_r - prev_res0_r;
                    encoder.encode(res1_r);
                    prev_res0_r = res0_r;
                    prev_res1_r = res1_r;

                    res0_l = buf_l;
                    res1_l = res0_l - prev_res0_l;
                    encoder.encode(res1_l);
                    prev_res0_l = res0_l;
                    prev_res1_l = res1_l;
                }
                else if(i == 4) {
                    res0_r = buf_r;
                    res1_r = res0_r - prev_res0_r;
                    res2_r = res1_r - prev_res1_r;
                    encoder.encode(res2_r);
                    prev_res0_r = res0_r;
                    prev_res1_r = res1_r;
                    prev_res2_r = res2_r;

                    res0_l = buf_l;
                    res1_l = res0_l - prev_res0_l;
                    res2_l = res1_l - prev_res1_l;
                    encoder.encode(res2_l);
                    prev_res0_l = res0_l;
                    prev_res1_l = res1_l;
                    prev_res2_l = res2_l;

                    firstFrameFlag = 0;
                }

            }
            else {
                res0_r = buf_r;
                res1_r = res0_r - prev_res0_r;
                res2_r = res1_r - prev_res1_r;
                res3_r = res2_r - prev_res2_r;
                encoder.encode(res3_r);
                prev_res0_r = res0_r;
                prev_res1_r = res1_r;
                prev_res2_r = res2_r;

                res0_l = buf_l;
                res1_l = res0_l - prev_res0_l;
                res2_l = res1_l - prev_res1_l;
                res3_l = res2_l - prev_res2_l;
                encoder.encode(res3_l);
                prev_res0_l = res0_l;
                prev_res1_l = res1_l;
                prev_res2_l = res2_l;
            }
        }
    }
    encoder.close();
}

void AudioCodec::decompress(std::string compressedFile, std::string outputFile) {

    GolombDecoder decoder(this->m, compressedFile);

    SndfileHandle sndFileOut { outputFile, SFM_WRITE, format, nChannels, sampleRate };

    if(sndFileOut.error()) {
        std::cerr << "Error: invalid output file" << std::endl;
    }

    std::vector<short> samplesOut(FRAMES_BUFFER_SIZE * nChannels);

    signed long val_r, val_l, prev_val_r,  prev_val_l;
    signed long val1_r, val1_l, prev_val1_r, prev_val1_l;
    signed long val2_r, val2_l, prev_val2_r, prev_val2_l;
    signed long val3_r, val3_l;
    unsigned int samplesRead = 0;

    val_r = decoder.decode();
    prev_val_r = val_r;
    samplesOut[samplesRead++] = val_r;

    val_l = decoder.decode();
    prev_val_l = val_l;
    samplesOut[samplesRead++] = val_l;

    val1_r = decoder.decode();
    prev_val1_r = val1_r;
    val_r = val1_r + prev_val_r;
    prev_val_r = val_r;
    samplesOut[samplesRead++] = val_r;

    val1_l = decoder.decode();
    prev_val1_l = val1_l;
    val_l = val1_l + prev_val_l;
    prev_val_l = val_l;
    samplesOut[samplesRead++] = val_l;

    val2_r = decoder.decode();
    prev_val2_r = val2_r;
    val1_r = val2_r + prev_val1_r;
    prev_val1_r = val1_r;
    val_r = val1_r + prev_val_r;
    prev_val_r = val_r;
    samplesOut[samplesRead++] = val_r;

    val2_l = decoder.decode();
    prev_val2_l = val2_l;
    val1_l = val2_l + prev_val1_l;
    prev_val1_l = val1_l;
    val_l = val1_l + prev_val_l;
    prev_val_l = val_l;
    samplesOut[samplesRead++] = val_l;

    unsigned int framesRead = 0;
    unsigned int blocksRead = 0;
    while(framesRead < nFrames - 3) {

        val3_r = decoder.decode();
        val2_r = val3_r + prev_val2_r;
        prev_val2_r = val2_r;
        val1_r = val2_r + prev_val1_r;
        prev_val1_r = val1_r;
        val_r = val1_r + prev_val_r;
        prev_val_r = val_r;

        val3_l = decoder.decode();
        val2_l = val3_l + prev_val2_l;
        prev_val2_l = val2_l;
        val1_l = val2_l + prev_val1_l;
        prev_val1_l = val1_l;
        val_l = val1_l + prev_val_l;
        prev_val_l = val_l;

        samplesOut[samplesRead++] = val_r;
        samplesOut[samplesRead++] = val_l;

        if(samplesRead >= FRAMES_BUFFER_SIZE * nChannels) {
            sndFileOut.writef(samplesOut.data(), FRAMES_BUFFER_SIZE);
            samplesRead = 0;
            blocksRead++;
        } else if(blocksRead == ((nFrames/FRAMES_BUFFER_SIZE)) && samplesRead >= lastFrameSize*nChannels) {
            sndFileOut.writef(samplesOut.data(), lastFrameSize);
        }
        framesRead++;
    }
    decoder.close();
}
