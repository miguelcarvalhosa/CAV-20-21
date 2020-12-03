
#include "AudioCodec.h"


AudioCodec::AudioCodec() {

}

AudioCodec::~AudioCodec() {

}

void AudioCodec::compress(std::string inputFile, std::string compressedFile, unsigned int m, audioCodec_ChannelRedundancy redundancy, audioCodec_parameterEstimationMode estimation, unsigned int estimation_nBlocks, audioCodec_lossMode loss, unsigned int lostBits) {
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
    initial_m = m;
    this->redundancy = redundancy;
    this->estimation = estimation;
    this->estimation_nBlocks = estimation_nBlocks;
    this->loss = loss;
    this->lostBits = lostBits;

    if(this->loss == LOSS_LOSSY) {
        if (this->lostBits > 16) {
            std::cerr << "Error: the number of bits to be lost is invalid" << std::endl;
        }
    }

    GolombEncoder encoder(initial_m, compressedFile);

    std::vector<short> samplesIn(FRAMES_BUFFER_SIZE * nChannels);
    int framesRead = 0;
    unsigned char firstFrameFlag = 1;
    unsigned int frameCount = 0;
    unsigned int estimatedBlocks = 0;

    short buf_x, buf_y;
    short res0_x, res0_y, prev_res0_x, prev_res0_y;
    short res1_x, res1_y, prev_res1_x, prev_res1_y;
    short res2_x, res2_y, prev_res2_x, prev_res2_y;
    short res3_x, res3_y;
    unsigned short res_3_mod_x, res_3_mod_y;
    unsigned long sum = 0;
    std::vector<signed long> X_Y(2);

    while (framesRead = sndFileIn.readf(samplesIn.data(), FRAMES_BUFFER_SIZE)) {
        frameCount++;
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
                    if (this->loss == LOSS_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    encoder.encode(res0_x);
                    prev_res0_x = res0_x;
                    res0_y = buf_y;

                    if (this->loss == LOSS_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }
                    encoder.encode(res0_y);
                    prev_res0_y = res0_y;

                }
                else if(i == 2) {
                    res0_x = buf_x;
                    if (this->loss == LOSS_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    res1_x = res0_x - prev_res0_x;
                    encoder.encode(res1_x);
                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;

                    res0_y = buf_y;
                    if (this->loss == LOSS_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }
                    res1_y = res0_y - prev_res0_y;
                    encoder.encode(res1_y);
                    prev_res0_y = res0_y;
                    prev_res1_y = res1_y;

                }
                else if(i == 4) {
                    res0_x = buf_x;
                    if (this->loss == LOSS_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    res1_x = res0_x - prev_res0_x;
                    res2_x = res1_x - prev_res1_x;
                    encoder.encode(res2_x);
                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;
                    prev_res2_x = res2_x;

                    res0_y = buf_y;
                    if (this->loss == LOSS_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }
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
                if (this->loss == LOSS_LOSSY) {
                    res0_x = res0_x >> this->lostBits;
                }
                res1_x = res0_x - prev_res0_x;
                res2_x = res1_x - prev_res1_x;
                res3_x = res2_x - prev_res2_x;
                encoder.encode(res3_x);
                prev_res0_x = res0_x;
                prev_res1_x = res1_x;
                prev_res2_x = res2_x;

                res0_y = buf_y;
                if (this->loss == LOSS_LOSSY) {
                    res0_y = res0_y >> this->lostBits;
                }
                res1_y = res0_y - prev_res0_y;
                res2_y = res1_y - prev_res1_y;
                res3_y = res2_y - prev_res2_y;
                encoder.encode(res3_y);
                prev_res0_y = res0_y;
                prev_res1_y = res1_y;
                prev_res2_y = res2_y;
               // std::cout << "i: " << i << " res3_x: " << res3_x << " res3_y: " << res3_y<< std::endl;
            }
            if(this->estimation == ESTIMATION_ADAPTATIVE) {
                if((i > 4 && firstFrameFlag == 0) || (frameCount>1)) {
                    estimatedBlocks++;
                    /* convert samples values to a positive range in order to perform the M estimation */
                    res_3_mod_x = res3_x > 0    ?   2 * res3_x - 1  :   -2 * res3_x;
                    res_3_mod_y = res3_y > 0    ?   2 * res3_y - 1  :   -2 * res3_y;
                    sum = sum + res_3_mod_x + res_3_mod_y;
                    if(estimatedBlocks == this->estimation_nBlocks) {
                        this->m = estimateM_fromBlock(sum, this->estimation_nBlocks);
                        //std::cout << "this->m: " << this->m << std::endl;
                        encoder.update(this->m);
                        //std::cout << "sum: " << sum << std::endl;
                        /* std::cout << "m updated to " << this->m << std::endl;*/
                        sum = 0;
                        estimatedBlocks = 0;
                    }
                }
            }

        }
    }
    encoder.close();
}

void AudioCodec::decompress(std::string compressedFile, std::string outputFile) {

    GolombDecoder decoder(initial_m, compressedFile);

    SndfileHandle sndFileOut { outputFile, SFM_WRITE, format, nChannels, sampleRate };

    if(sndFileOut.error()) {
        std::cerr << "Error: invalid output file" << std::endl;
    }

    std::vector<short> samplesOut(FRAMES_BUFFER_SIZE * nChannels);

    signed long val_x, val_y, prev_val_x,  prev_val_y;
    signed long val1_x, val1_y, prev_val1_x, prev_val1_y;
    signed long val2_x, val2_y, prev_val2_x, prev_val2_y;
    signed long val3_x, val3_y;
    unsigned short val3_mod_x, val3_mod_y;
    std::vector<signed long> R_L(2);
    unsigned int samplesRead = 0;
    unsigned int estimatedBlocks = 0;
    unsigned long sum = 0;

    val_x = decoder.decode();
    prev_val_x = val_x;
    if(this->loss == LOSS_LOSSY) {
        val_x = val_x << this->lostBits;
    }
    val_y = decoder.decode();
    prev_val_y = val_y;
    if(this->loss == LOSS_LOSSY) {
        val_y = val_y << this->lostBits;
    }
    R_L = calculateR_L(val_x, val_y, redundancy);
    samplesOut[samplesRead++] = R_L[0];
    samplesOut[samplesRead++] = R_L[1];

    val1_x = decoder.decode();
    prev_val1_x = val1_x;
    val_x = val1_x + prev_val_x;
    prev_val_x = val_x;
    if(this->loss == LOSS_LOSSY) {
        val_x = val_x << this->lostBits;
    }
    val1_y = decoder.decode();
    prev_val1_y = val1_y;
    val_y = val1_y + prev_val_y;
    prev_val_y = val_y;
    if(this->loss == LOSS_LOSSY) {
        val_y = val_y << this->lostBits;
    }
    R_L = calculateR_L(val_x, val_y, redundancy);
    samplesOut[samplesRead++] = R_L[0];
    samplesOut[samplesRead++] = R_L[1];

    val2_x = decoder.decode();
    prev_val2_x = val2_x;
    val1_x = val2_x + prev_val1_x;
    prev_val1_x = val1_x;
    val_x = val1_x + prev_val_x;
    prev_val_x = val_x;
    if(this->loss == LOSS_LOSSY) {
        val_x = val_x << this->lostBits;
    }
    val2_y = decoder.decode();
    prev_val2_y = val2_y;
    val1_y = val2_y + prev_val1_y;
    prev_val1_y = val1_y;
    val_y = val1_y + prev_val_y;
    prev_val_y = val_y;
    if(this->loss == LOSS_LOSSY) {
        val_y = val_y << this->lostBits;
    }
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
        if(this->loss == LOSS_LOSSY) {
            val_x = val_x << this->lostBits;
        }
        val3_y = decoder.decode();
        val2_y = val3_y + prev_val2_y;
        prev_val2_y = val2_y;
        val1_y = val2_y + prev_val1_y;
        prev_val1_y = val1_y;
        val_y = val1_y + prev_val_y;
        prev_val_y = val_y;
        if(this->loss == LOSS_LOSSY) {
            val_y = val_y << this->lostBits;
        }
        R_L = calculateR_L(val_x, val_y, redundancy);
        samplesOut[samplesRead++] = R_L[0];
        samplesOut[samplesRead++] = R_L[1];

        estimatedBlocks++;
        val3_mod_x = val3_x >0?  2*val3_x -1 : -2*val3_x;
        val3_mod_y = val3_y >0?  2*val3_y -1 : -2*val3_y;
        sum = sum + val3_mod_x + val3_mod_y;
        if(estimation == ESTIMATION_ADAPTATIVE) {
            if (estimatedBlocks == estimation_nBlocks) {
                m = estimateM_fromBlock(sum, estimation_nBlocks);
                decoder.update(m);
                /*std::cout << "sum: " << sum << std::endl;
                std::cout << "descodificador m updated to " << m << std::endl;*/
                sum = 0;
                estimatedBlocks = 0;
            }
        }

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


unsigned int AudioCodec::estimateM(std::string inputFile, audioCodec_ChannelRedundancy redundancy, audioCodec_lossMode loss, unsigned int lostBits) {

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

    this->redundancy = redundancy;
    this->loss = loss;
    this->lostBits = lostBits;

    /* vectors to store the samples from each channel and the calculated mono version */
    std::vector<short> samplesIn(FRAMES_BUFFER_SIZE * nChannels);
    std::vector<signed long> X_Y(2);
    short buf_x, buf_y;
    short res0_x, res0_y, prev_res0_x, prev_res0_y;
    short res1_x, res1_y, prev_res1_x, prev_res1_y;
    short res2_x, res2_y, prev_res2_x, prev_res2_y;
    short res3_x, res3_y;

    unsigned char firstFrameFlag = 1;
    int framesRead = 0;
    unsigned int frameCount = 0;

    unsigned short res_3_mod_x=0, res_3_mod_y=0;
    unsigned long sum = 0;
    unsigned int initial_m;

    while (framesRead = sndFileIn.readf(samplesIn.data(), FRAMES_BUFFER_SIZE)) {
        frameCount++;
        for(unsigned int i=0; i < framesRead*nChannels; i += nChannels) {
            X_Y = calculateX_Y(samplesIn[i], samplesIn[i + 1], this->redundancy);
            buf_x = X_Y[0];
            buf_y = X_Y[1];
            if (firstFrameFlag == 1) {
                if (i == 0) {
                    res0_x = buf_x;
                    if (this->loss == LOSS_LOSSY) {
                        res0_x = res0_x >> this->lostBits;

                    }

                    prev_res0_x = res0_x;
                    res0_y = buf_y;
                    if (this->loss == LOSS_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }

                    prev_res0_y = res0_y;

                } else if (i == 2) {
                    res0_x = buf_x;
                    if (this->loss == LOSS_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    res1_x = res0_x - prev_res0_x;

                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;

                    res0_y = buf_y;
                    if (this->loss == LOSS_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }
                    res1_y = res0_y - prev_res0_y;

                    prev_res0_y = res0_y;
                    prev_res1_y = res1_y;

                } else if (i == 4) {
                    res0_x = buf_x;
                    if (this->loss == LOSS_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    res1_x = res0_x - prev_res0_x;
                    res2_x = res1_x - prev_res1_x;

                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;
                    prev_res2_x = res2_x;

                    res0_y = buf_y;

                    if (this->loss == LOSS_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }
                    res1_y = res0_y - prev_res0_y;
                    res2_y = res1_y - prev_res1_y;

                    prev_res0_y = res0_y;
                    prev_res1_y = res1_y;
                    prev_res2_y = res2_y;

                    firstFrameFlag = 0;

                }

            } else {
                res0_x = buf_x;
                if (this->loss == LOSS_LOSSY) {
                    res0_x = res0_x >> this->lostBits;
                }
                res1_x = res0_x - prev_res0_x;
                res2_x = res1_x - prev_res1_x;
                res3_x = res2_x - prev_res2_x;

                prev_res0_x = res0_x;
                prev_res1_x = res1_x;
                prev_res2_x = res2_x;

                res0_y = buf_y;
                if (this->loss == LOSS_LOSSY) {
                    res0_y = res0_y >> this->lostBits;
                }
                res1_y = res0_y - prev_res0_y;
                res2_y = res1_y - prev_res1_y;
                res3_y = res2_y - prev_res2_y;

                prev_res0_y = res0_y;
                prev_res1_y = res1_y;
                prev_res2_y = res2_y;
                //std::cout << "i: " << i << " res3_x: " << res3_x << " res3_y: " << res3_y<< std::endl;
            }
            if((i > 4 && firstFrameFlag == 0) || (frameCount>1)) {
                //std::cout << "i: " << i << " res3_x: " << res3_x << " res3_y: " << res3_y<< std::endl;
                /* convert samples values to a positive range in order to perform the M estimation */
                res_3_mod_x = res3_x > 0    ?   2 * res3_x - 1  :   -2 * res3_x;
                res_3_mod_y = res3_y > 0    ?   2 * res3_y - 1  :   -2 * res3_y;
                sum = sum + res_3_mod_x + res_3_mod_y;

            }
        }
    }
    initial_m = estimateM_fromBlock(sum, sndFileIn.frames());
    //std::cout << "sum: " << sum << std::endl;
    //std::cout << "initial_m: " << initial_m << std::endl;

    return initial_m;
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


unsigned int AudioCodec::estimateM_fromBlock(unsigned int sum, unsigned int blockSize) {
    double mean, alfa;
    unsigned int m;

    mean = (sum/(double)(blockSize*2));
    alfa = mean/(1+mean);
    m = ceil(-1/log2(alfa));
    if(m<2) {
        m = 2;
    }
    return m;
}
