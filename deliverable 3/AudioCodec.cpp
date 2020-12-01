

#include "AudioCodec.h"


AudioCodec::AudioCodec() {

}

AudioCodec::~AudioCodec() {

}

void AudioCodec::compress(std::string inputFile, std::string compressedFile, unsigned int m, audioCodec_ChannelRedundancy redundancy, audioCodec_parameterEstimationMode estimation) {
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
    this->redundancy = redundancy;

    GolombEncoder encoder(this->m, compressedFile);


    std::vector<short> samplesIn(FRAMES_BUFFER_SIZE * nChannels);
    int framesRead = 0;
    unsigned char firstFrameFlag = 1;

    short buf_x, buf_y;
    short res0_x, res0_y, prev_res0_x, prev_res0_y;
    short res1_x, res1_y, prev_res1_x, prev_res1_y;
    short res2_x, res2_y, prev_res2_x, prev_res2_y;
    short res3_x, res3_y;
    std::vector<signed long> X_Y(2);

    while (framesRead = sndFileIn.readf(samplesIn.data(), FRAMES_BUFFER_SIZE)) {
        if(framesRead != FRAMES_BUFFER_SIZE) {
            lastFrameSize = framesRead;
        }

        for(unsigned int i=0; i < framesRead*nChannels; i += nChannels) {
            X_Y = calculateX_Y (samplesIn[i], samplesIn[i+1], this->redundancy);
            buf_x = X_Y[0];
            buf_y = X_Y[1];
            if(firstFrameFlag == 1) {
                if(i == 0) {
                    res0_x = buf_x;
                    encoder.encode(res0_x);
                    prev_res0_x = res0_x;
                    res0_y = buf_y;
                    encoder.encode(res0_y);
                    prev_res0_y = res0_y;
                }
                else if( i == 2) {
                    res0_x = buf_x;
                    res1_x = res0_x - prev_res0_x;
                    encoder.encode(res1_x);
                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;

                    res0_y = buf_y;
                    res1_y = res0_y - prev_res0_y;
                    encoder.encode(res1_y);
                    prev_res0_y = res0_y;
                    prev_res1_y = res1_y;
                }
                else if(i == 4) {
                    res0_x = buf_x;
                    res1_x = res0_x - prev_res0_x;
                    res2_x = res1_x - prev_res1_x;
                    encoder.encode(res2_x);
                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;
                    prev_res2_x = res2_x;

                    res0_y = buf_y;
                    res1_y = res0_y - prev_res0_y;
                    res2_y = res1_y - prev_res1_y;
                    encoder.encode(res2_y);
                    prev_res0_y = res0_y;
                    prev_res1_y = res1_y;
                    prev_res2_y = res2_y;

                    firstFrameFlag = 0;
                }

            }
            else {
                res0_x = buf_x;
                res1_x = res0_x - prev_res0_x;
                res2_x = res1_x - prev_res1_x;
                res3_x = res2_x - prev_res2_x;
                encoder.encode(res3_x);
                prev_res0_x = res0_x;
                prev_res1_x = res1_x;
                prev_res2_x = res2_x;

                res0_y = buf_y;
                res1_y = res0_y - prev_res0_y;
                res2_y = res1_y - prev_res1_y;
                res3_y = res2_y - prev_res2_y;
                encoder.encode(res3_y);
                prev_res0_y = res0_y;
                prev_res1_y = res1_y;
                prev_res2_y = res2_y;
            }

        }
    }
    encoder.close();
}

void AudioCodec::decompress(std::string compressedFile, std::string outputFile) {

    GolombDecoder decoder(m, compressedFile);

    SndfileHandle sndFileOut { outputFile, SFM_WRITE, format, nChannels, sampleRate };

    if(sndFileOut.error()) {
        std::cerr << "Error: invalid output file" << std::endl;
    }

    std::vector<short> samplesOut(FRAMES_BUFFER_SIZE * nChannels);

    signed long val_x, val_y, prev_val_x,  prev_val_y;
    signed long val1_x, val1_y, prev_val1_x, prev_val1_y;
    signed long val2_x, val2_y, prev_val2_x, prev_val2_y;
    signed long val3_x, val3_y;
    std::vector<signed long> R_L(2);
    unsigned int samplesRead = 0;

    val_x = decoder.decode();
    prev_val_x = val_x;
    val_y = decoder.decode();
    prev_val_y = val_y;
    R_L = calculateR_L(val_x, val_y, redundancy);
    samplesOut[samplesRead++] = R_L[0];
    samplesOut[samplesRead++] = R_L[1];

    val1_x = decoder.decode();
    prev_val1_x = val1_x;
    val_x = val1_x + prev_val_x;
    prev_val_x = val_x;
    val1_y = decoder.decode();
    prev_val1_y = val1_y;
    val_y = val1_y + prev_val_y;
    prev_val_y = val_y;
    R_L = calculateR_L(val_x, val_y, redundancy);
    samplesOut[samplesRead++] = R_L[0];
    samplesOut[samplesRead++] = R_L[1];

    val2_x = decoder.decode();
    prev_val2_x = val2_x;
    val1_x = val2_x + prev_val1_x;
    prev_val1_x = val1_x;
    val_x = val1_x + prev_val_x;
    prev_val_x = val_x;
    val2_y = decoder.decode();
    prev_val2_y = val2_y;
    val1_y = val2_y + prev_val1_y;
    prev_val1_y = val1_y;
    val_y = val1_y + prev_val_y;
    prev_val_y = val_y;
    R_L = calculateR_L(val_x, val_y, redundancy);
    samplesOut[samplesRead++] = R_L[0];
    samplesOut[samplesRead++] = R_L[1];

    unsigned int framesRead = 0;
    unsigned int blocksRead = 0;
    while(framesRead < nFrames - 3) {

        val3_x = decoder.decode();
        val2_x = val3_x + prev_val2_x;
        prev_val2_x = val2_x;
        val1_x = val2_x + prev_val1_x;
        prev_val1_x = val1_x;
        val_x = val1_x + prev_val_x;
        prev_val_x = val_x;
        val3_y = decoder.decode();
        val2_y = val3_y + prev_val2_y;
        prev_val2_y = val2_y;
        val1_y = val2_y + prev_val1_y;
        prev_val1_y = val1_y;
        val_y = val1_y + prev_val_y;
        prev_val_y = val_y;
        R_L = calculateR_L(val_x, val_y, redundancy);
        samplesOut[samplesRead++] = R_L[0];
        samplesOut[samplesRead++] = R_L[1];

        if(samplesRead >= FRAMES_BUFFER_SIZE * nChannels) {
            sndFileOut.writef(samplesOut.data(), FRAMES_BUFFER_SIZE);
            samplesRead = 0;
            blocksRead++;
        }
        else if(blocksRead == ((nFrames/FRAMES_BUFFER_SIZE)) && samplesRead >= lastFrameSize*nChannels) {
            sndFileOut.writef(samplesOut.data(), lastFrameSize);
        }
        framesRead++;
    }
    decoder.close();
}


std::vector<signed long> AudioCodec::calculateR_L (signed long val_x, signed long val_y, audioCodec_ChannelRedundancy redundancy) {
    std::vector<signed long> R_L(2);
    signed long val_l, val_r;
    if(redundancy == REDUNDANCY_MID_SIDE) {
        val_l = (2*val_x + val_y)/2;
        val_r = val_l - val_y;
    } else if(redundancy == REDUNDANCY_RIGHT_SIDE) {
        val_r = val_x;
        val_l = val_r + val_y;
    } else if(redundancy == REDUNDANCY_LEFT_SIDE){
        val_l = val_x;
        val_r = val_l - val_y;
    } else {
        /* Default is Independent Mode */
        val_r = val_x;
        val_l = val_y;
    }
    R_L[0] = val_r;
    R_L[1] = val_l;
    return R_L;
}


std::vector<long signed> AudioCodec::calculateX_Y (signed long val_r, signed long val_l, audioCodec_ChannelRedundancy redundancy) {
    std::vector<signed long> X_Y(2);
    signed long val_x, val_y;
    if(redundancy == REDUNDANCY_MID_SIDE) {
        val_x = (val_r + val_l) / 2;    // code (L+R)/2
        val_y = (val_l - val_r);        // code L-R
    } else if(redundancy == REDUNDANCY_RIGHT_SIDE) {
        val_x = val_r;                  // code R
        val_y = (val_l - val_r);        // code L-R
    } else if(redundancy == REDUNDANCY_LEFT_SIDE){
        val_x = val_l;                  // code L
        val_y = (val_l - val_r);        // code L-R
    } else {
        /* Default is Independent Mode */
        val_x = val_r;
        val_y = val_l;
    }
    X_Y[0] = val_x;
    X_Y[1] = val_y;
    return X_Y;
}
