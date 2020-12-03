
#include "AudioCodec.h"


AudioCodec::AudioCodec() {

}

AudioCodec::~AudioCodec() {

}

void AudioCodec::compress(std::string inputFile, std::string compressedFile, unsigned int m, audioCodec_ChannelRedundancy redundancy, audioCodec_parameterEstimationMode estimation, unsigned int estimation_nBlocks, audioCodec_lossMode loss, unsigned int lostBits,  audioCodec_histogramMode histMode, std::string histogramFileName) {
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

    if(this->loss == MODE_LOSSY) {
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
    unsigned short res_3_mod_x;
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
                    if (this->loss == MODE_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    encoder.encode(res0_x);
                    prev_res0_x = res0_x;
                    res0_y = buf_y;

                    if (this->loss == MODE_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }
                    encoder.encode(res0_y);
                    prev_res0_y = res0_y;

                }
                else if(i == 2) {
                    res0_x = buf_x;
                    if (this->loss == MODE_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    res1_x = res0_x - prev_res0_x;
                    encoder.encode(res1_x);
                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;

                    res0_y = buf_y;
                    if (this->loss == MODE_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }
                    res1_y = res0_y - prev_res0_y;
                    encoder.encode(res1_y);
                    prev_res0_y = res0_y;
                    prev_res1_y = res1_y;

                }
                else if(i == 4) {
                    res0_x = buf_x;
                    if (this->loss == MODE_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    res1_x = res0_x - prev_res0_x;
                    res2_x = res1_x - prev_res1_x;
                    encoder.encode(res2_x);
                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;
                    prev_res2_x = res2_x;

                    res0_y = buf_y;
                    if (this->loss == MODE_LOSSY) {
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
                if (this->loss == MODE_LOSSY) {
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
                if (this->loss == MODE_LOSSY) {
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
                    sum = sum + res_3_mod_x;
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
            if(histMode == MODE_RESIDUAL_HISTOGRAM) {
                calculateResHist(histogramFileName, res3_x, res3_y, nFrames);
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
    unsigned short val3_mod_x;
    std::vector<signed long> R_L(2);
    unsigned int samplesRead = 0;
    unsigned int estimatedBlocks = 0;
    unsigned long sum = 0;

    val_x = decoder.decode();
    prev_val_x = val_x;
    if(this->loss == MODE_LOSSY) {
        val_x = val_x << this->lostBits;
    }
    val_y = decoder.decode();
    prev_val_y = val_y;
    if(this->loss == MODE_LOSSY) {
        val_y = val_y << this->lostBits;
    }
    R_L = calculateR_L(val_x, val_y, redundancy);
    samplesOut[samplesRead++] = R_L[0];
    samplesOut[samplesRead++] = R_L[1];

    val1_x = decoder.decode();
    prev_val1_x = val1_x;
    val_x = val1_x + prev_val_x;
    prev_val_x = val_x;
    if(this->loss == MODE_LOSSY) {
        val_x = val_x << this->lostBits;
    }
    val1_y = decoder.decode();
    prev_val1_y = val1_y;
    val_y = val1_y + prev_val_y;
    prev_val_y = val_y;
    if(this->loss == MODE_LOSSY) {
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
    if(this->loss == MODE_LOSSY) {
        val_x = val_x << this->lostBits;
    }
    val2_y = decoder.decode();
    prev_val2_y = val2_y;
    val1_y = val2_y + prev_val1_y;
    prev_val1_y = val1_y;
    val_y = val1_y + prev_val_y;
    prev_val_y = val_y;
    if(this->loss == MODE_LOSSY) {
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
        if(this->loss == MODE_LOSSY) {
            val_x = val_x << this->lostBits;
        }
        val3_y = decoder.decode();
        val2_y = val3_y + prev_val2_y;
        prev_val2_y = val2_y;
        val1_y = val2_y + prev_val1_y;
        prev_val1_y = val1_y;
        val_y = val1_y + prev_val_y;
        prev_val_y = val_y;
        if(this->loss == MODE_LOSSY) {
            val_y = val_y << this->lostBits;
        }
        R_L = calculateR_L(val_x, val_y, redundancy);
        samplesOut[samplesRead++] = R_L[0];
        samplesOut[samplesRead++] = R_L[1];

        estimatedBlocks++;
        val3_mod_x = val3_x >0?  2*val3_x -1 : -2*val3_x;
        sum = sum + val3_mod_x;
        if(estimation == ESTIMATION_ADAPTATIVE) {
            if (estimatedBlocks == estimation_nBlocks) {
                m = estimateM_fromBlock(sum, estimation_nBlocks);
                decoder.update(m);
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

    unsigned short res_3_mod_x=0;
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
                    if (this->loss == MODE_LOSSY) {
                        res0_x = res0_x >> this->lostBits;

                    }

                    prev_res0_x = res0_x;
                    res0_y = buf_y;
                    if (this->loss == MODE_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }

                    prev_res0_y = res0_y;

                } else if (i == 2) {
                    res0_x = buf_x;
                    if (this->loss == MODE_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    res1_x = res0_x - prev_res0_x;

                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;

                    res0_y = buf_y;
                    if (this->loss == MODE_LOSSY) {
                        res0_y = res0_y >> this->lostBits;
                    }
                    res1_y = res0_y - prev_res0_y;

                    prev_res0_y = res0_y;
                    prev_res1_y = res1_y;

                } else if (i == 4) {
                    res0_x = buf_x;
                    if (this->loss == MODE_LOSSY) {
                        res0_x = res0_x >> this->lostBits;
                    }
                    res1_x = res0_x - prev_res0_x;
                    res2_x = res1_x - prev_res1_x;

                    prev_res0_x = res0_x;
                    prev_res1_x = res1_x;
                    prev_res2_x = res2_x;

                    res0_y = buf_y;

                    if (this->loss == MODE_LOSSY) {
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
                if (this->loss == MODE_LOSSY) {
                    res0_x = res0_x >> this->lostBits;
                }
                res1_x = res0_x - prev_res0_x;
                res2_x = res1_x - prev_res1_x;
                res3_x = res2_x - prev_res2_x;

                prev_res0_x = res0_x;
                prev_res1_x = res1_x;
                prev_res2_x = res2_x;

                res0_y = buf_y;
                if (this->loss == MODE_LOSSY) {
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
                sum = sum + res_3_mod_x;

            }
        }
    }
    initial_m = estimateM_fromBlock(sum, sndFileIn.frames());
    //std::cout << "sum: " << sum << std::endl;
    //std::cout << "initial_m: " << initial_m << std::endl;

    return initial_m;
}


void AudioCodec::calculateAudioHist(std::string histFileName, std::string audioFileName) {

    /* vectors to store the element count of each channel and the mono version */
    std::vector<long unsigned> hist_r((int)pow(2,16)), hist_l((int)pow(2,16)), hist_mono((int)pow(2,16));

    /* file for storing calculated frequencies */
    std::ofstream outfile(histFileName);

    SndfileHandle sndFileIn { audioFileName };
    if(sndFileIn.error()) {
        std::cerr << "Error: invalid input file" << std::endl;
    }

    if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        std::cerr << "Error: file is not in WAV format" << std::endl;
    }

    if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        std::cerr << "Error: file is not in PCM_16 format" << std::endl;
    }

    std::vector<short> samples(FRAMES_BUFFER_SIZE * sndFileIn.channels());

    /* reads all frame from source audio file and computes for each channel the element count */
    while (sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        for(unsigned int i=0; i<samples.size(); i+=sndFileIn.channels()) {
            /* NOTE: The addition of the 32767 constant is only needed to convert the negative
             *       samples into positive indexes that can be accessed in the hist vectors to
             *       count each element occurrence */
            hist_r[samples[i]+32767] += 1;
            hist_l[samples[i+1]+32767] += 1;
            hist_mono[((samples[i] + samples[i+1])/2) + 32767] += 1;
        }
    }
    /* write the computed element count */
    for(unsigned int k=0; k< hist_r.size(); k++) {
        outfile << hist_r [k] << " " << hist_l[k] << " " << hist_mono[k] << std::endl;
    }
    outfile.close();
}


std::vector<float> AudioCodec::audioEntropy(std::string fileName) {
    // nº de bits médio/simbolo
    float Pi_r, Pi_l, Pi_mono;
    float ent_r = 0, ent_l = 0, ent_mono = 0;
    unsigned long int count_r = 0, count_l = 0, count_mono = 0;
    std::vector<float> ent(3);

    std::vector<unsigned int> hist_r(FRAMES_BUFFER_SIZE), hist_l(FRAMES_BUFFER_SIZE), hist_mono(FRAMES_BUFFER_SIZE);

    std::ifstream hist_file(fileName);

    unsigned int i=0;
    while(!hist_file.eof()){
        hist_file >> hist_r[i];
        hist_file >> hist_l[i];
        hist_file >> hist_mono[i];
        i++;
    }
    count_r = accumulate(hist_r.begin(), hist_r.end(), 0);
    count_l = accumulate(hist_l.begin(), hist_l.end(), 0);
    count_mono = accumulate(hist_mono.begin(), hist_mono.end(), 0);

    for (i=0; i<FRAMES_BUFFER_SIZE; i++) {
        if(hist_r[i]> 0) {
            Pi_r = (float)(hist_r[i])/count_r;
            ent_r -= Pi_r*log2(Pi_r);
        }
        if(hist_l[i]> 0) {
            Pi_l = (float)(hist_l[i])/count_l;
            ent_l -= Pi_l*log2(Pi_l);
        }
        if(hist_mono[i]> 0) {
            Pi_mono= (float)(hist_mono[i])/count_mono;
            ent_mono -= Pi_mono*log2(Pi_mono);
        }
    }
    ent[0] = ent_r;
    ent[1] = ent_l;
    ent[2] = ent_mono;

    hist_file.close();
    return ent;
}


std::vector<float> AudioCodec::residualsEntropy(std::string fileName) {
    float Pi_x, Pi_y;
    float ent_x = 0, ent_y = 0;
    unsigned long int count_x = 0, count_y = 0;

    std::vector<unsigned int> hist_res_x(pow(2,16)), hist_res_y(pow(2,16));
    std::vector<float> ent(2);
    std::ifstream hist_file(fileName);

    unsigned int i=0;
    while(!hist_file.eof()){
        hist_file >> hist_res_x[i];
        hist_file >> hist_res_y[i];
        i++;
    }
    count_x = accumulate(hist_res_x.begin(), hist_res_x.end(), 0);
    count_y = accumulate(hist_res_y.begin(), hist_res_y.end(), 0);

    for (i=0;i<FRAMES_BUFFER_SIZE;i++) {
        if (hist_res_x[i] > 0) {
            Pi_x = (float) (hist_res_x[i]) / count_x;
            ent_x -= Pi_x * log2(Pi_x);
        }
        if (hist_res_y[i] > 0) {
            Pi_y = (float) (hist_res_y[i]) / count_y;
            ent_y -= Pi_y * log2(Pi_y);
        }

    }
    ent[0] = ent_x;
    ent[1] = ent_y;
    return ent;
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
        /* The default is Independent Mode */
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

    mean = (sum/(double)(blockSize));
    alfa = mean/(1+mean);
    m = ceil(-1/log2(alfa));
    if(m<2) {
        m = 2;
    }
    return m;
}


void AudioCodec::calculateResHist(std::string fileName, short res_x, short res_y, unsigned int totalFrames) {
    /* vector to store the element count of the histogram */
    static std::vector<unsigned long> hist_res_x((int)pow(2,16)), hist_res_y((int)pow(2,16));
    /* count the number of residuals that have been written */
    static unsigned int nSample=0;
    if(nSample==0) {
        std::cout << "Started Residuals histogram computation" << std::endl;
    }
    nSample++;
    /* file for writing the residuals histograms for each predictor order */
    std::ofstream outfile(fileName);
    hist_res_x[res_x+32767] += 1;
    hist_res_y[res_y+32767] += 1;

    if(nSample == totalFrames) {
        std::cout << "Residuals histogram computation done. Writting destination file..." << std::endl;
        for(unsigned int k=0; k < hist_res_x.size(); k++) {
            outfile << hist_res_x[k] << " " << hist_res_y[k] << std::endl;
        }
    }
}
