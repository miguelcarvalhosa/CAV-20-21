
/**
 * \file point14.cpp
 *
 * \brief Contains the resolution of point 14 of Deliverable 3.
 *        This program reads a .wav audio file
 *
 *
 *        To use the program, the user must pass two arguments: the input audio file
 *        path and the ouput residuals file path
 *
 *        Usage: point14 <input audio file> <output residuals file>
 *
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */

#include <iostream>
#include <vector>
#include <sndfile.hh>
#include "math.h"
#include <numeric>
#include "GolombEncoder.h"
#include "GolombDecoder.h"

using namespace std;

vector<float> audioEntropy(string fileName);
void calculateAudioHist(string histFileName, string audioFileName);

vector<float> residualsEntropy(string fileName);
void calculateResHist(string fileName, short res_x, short res_y, unsigned int totalFrames);

vector<long signed> calculateX_Y (signed long val_r, signed long val_l, string channelRedudancy_mode);
vector<long signed> calculateR_L (signed long val_x, signed long val_y, string channelRedudancy_mode);

unsigned int estimateM_fromBlock(unsigned int sum, unsigned int blockSize);
unsigned int estimateM_fromAllSamples(string audioFileName);

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

/* The coded has four different modes of operation depending on the type of channel redudancy:
 * "Mid-side", "Right-side", "Left-side" & "Independent" (default) */
string channelRedudancy_mode = "Independent";
string parameterEstimation_mode = "Block"; //AllSamples

int main(int argc, char *argv[]) {
    /* vectors to store the samples from each channel and the calculated mono version */
    vector<short> samples(FRAMES_BUFFER_SIZE * 2);

    vector<short> r_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_1(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_2(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_3(FRAMES_BUFFER_SIZE * 2);
    unsigned short res_3_mod_x=0, res_3_mod_y=0;

    //ofstream outfile1("original.txt");
    //ofstream outfile2("decoded.txt");
    /* identifies the first frame of data */
    unsigned int frame = 0;

    if(argc < 3) {
        cerr << "Usage: wavcp <input file.wav> <output file.txt>" << endl;
        return 1;
    }

    SndfileHandle sndFileIn { argv[argc-2] };
    if(sndFileIn.error()) {
        cerr << "Error: invalid input file" << endl;
        return 1;
    }

    if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format" << endl;
        return 1;
    }

    if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format" << endl;
        return 1;
    }

    cout << "Input file has:" << endl;
    cout << '\t' << sndFileIn.frames() << " frames" << endl;
    cout << '\t' << sndFileIn.samplerate() << " samples per second" << endl;
    cout << '\t' << sndFileIn.channels() << " channels" << endl;


    int nFrames = 0;
    sf_count_t c;
    unsigned int lastFrameSize;
    unsigned int blockSize = 10;
    unsigned long sum=0;
    unsigned int m, initial_m=500;
    unsigned int nBlock=0;

    unsigned int count = 0;

    vector<signed long> X_Y(2);

    if(parameterEstimation_mode == "AllSamples") {
        initial_m = estimateM_fromAllSamples(argv[1]);
    }

    GolombEncoder encoder(initial_m,"Residuals.bin");
    /* reads all frame from source audio file and computes for each channel the element count */
    while (c=(sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE))) {
        nFrames++;
        if(c!=FRAMES_BUFFER_SIZE) {
            lastFrameSize = c;
        }
        for(int i=0; i< c*2; i=i+2) {

            X_Y = calculateX_Y (samples[i], samples[i+1], channelRedudancy_mode);
            //outfile1 << samples[i] << " " << samples[i+1] << endl;
            if(frame == 0) {
                if (i == 0) {

                    res_0[i] = X_Y[0];
                    res_0[i + 1] = X_Y[1];
                    encoder.encode(res_0[i]);
                    encoder.encode(res_0[i + 1]);

                } else if (i == 2) {

                    res_0[i] = X_Y[0];
                    res_1[i] = res_0[i] - res_0[i - 2];
                    encoder.encode(res_1[i]);

                    res_0[i + 1] = X_Y[1];
                    res_1[i + 1] = res_0[i + 1] - res_0[i - 1];
                    encoder.encode(res_1[i + 1]);


                } else if (i == 4) {

                    res_0[i] = X_Y[0];
                    res_1[i] = res_0[i] - res_0[i - 2];
                    res_2[i] = res_1[i] - res_1[i - 2];
                    encoder.encode(res_2[i]);

                    res_0[i + 1] = X_Y[1];
                    res_1[i + 1] = res_0[i + 1] - res_0[i - 1];
                    res_2[i + 1] = res_1[i + 1] - res_1[i - 1];
                    encoder.encode(res_2[i + 1]);
                    frame = 1;

                }
            } else if (i == 0) {

                res_0[i] = X_Y[0];
                res_1[i] = res_0[i] - res_0[samples.size() - 2];
                res_2[i] = res_1[i] - res_1[samples.size() - 2];
                res_3[i] = res_2[i] - res_2[samples.size() - 2];
                encoder.encode(res_3[i]);

                res_0[i + 1] = X_Y[1];
                res_1[i + 1] = res_0[i + 1] - res_0[samples.size() - 1];
                res_2[i + 1] = res_1[i + 1] - res_1[samples.size() - 1];
                res_3[i + 1] = res_2[i + 1] - res_2[samples.size() - 1];
                encoder.encode(res_3[i + 1]);
            } else {

                res_0[i] = X_Y[0];
                res_1[i] = res_0[i] - res_0[i-2];
                res_2[i] = res_1[i] - res_1[i-2];
                res_3[i] = res_2[i] - res_2[i-2];
                encoder.encode(res_3[i]);

                res_0[i+1] = X_Y[1];
                res_1[i+1] = res_0[i+1] - res_0[i-1];
                res_2[i+1] = res_1[i+1] - res_1[i-1];
                res_3[i+1] = res_2[i+1] - res_2[i-1];
                encoder.encode(res_3[i+1]);
            }
            if(parameterEstimation_mode == "Block") {
                if((frame == 1 && i>4) || nFrames>1 ) {
                    nBlock++;
                    /* convert samples values to a positive range in order to perform the M estimation */
                    res_3_mod_x = res_3[i]>0?  2*res_3[i] -1   : -2*res_3[i];
                    res_3_mod_y = res_3[i+1]>0?  2*res_3[i+1] -1   : -2*res_3[i+1];
                    sum = sum + res_3_mod_x + res_3_mod_y;

                    if(nBlock == blockSize) {
                        m = estimateM_fromBlock(sum, blockSize);
                        encoder.update(m);
                        /* cout << "sum: " << sum << endl;
                        cout << "m updated to " << m << endl;*/
                        sum = 0;
                        nBlock = 0;
                    }
                }
            }
            calculateResHist("hist_res_3.txt", res_3[i], res_3[i+1], sndFileIn.frames());
        }
    }
    //outfile1.close();
    cout << "---descodificador----" << endl;
    encoder.close();
    GolombDecoder decoder(initial_m,"Residuals.bin");

    signed long val_r,val_l, preval_x,  preval_y;
    signed long val_x, val_y, val1_x, val1_y, prev1_x, prev1_y;
    signed long val2_x, val2_y, prev2_x, prev2_y;
    signed long val3_x, val3_y;


    SndfileHandle sndFileOut { argv[argc-1], SFM_WRITE, sndFileIn.format(),
                               sndFileIn.channels(), sndFileIn.samplerate() };

    if(sndFileOut.error()) {
        cerr << "Error: invalid output file" << endl;
        return 1;
    }

    vector<short> samples_out(FRAMES_BUFFER_SIZE * sndFileIn.channels());
    vector<signed long> R_L(2);

    val_x = decoder.decode();
    preval_x = val_x;
    val_y = decoder.decode();
    preval_y = val_y;

    /* calculate the value of each channel from the received values x and y*/
    R_L = calculateR_L(val_x, val_y,channelRedudancy_mode);
    samples_out[0] = R_L[0];
    samples_out[1] = R_L[1];

    //outfile2 << R_L[0] << " " << R_L[1] << endl;

    val1_x = decoder.decode();
    prev1_x = val1_x;
    val_x = val1_x + preval_x;
    preval_x = val_x;

    val1_y = decoder.decode();
    prev1_y = val1_y;
    val_y = val1_y + preval_y;
    preval_y = val_y;

    /* calculate the value of each channel from the received values x and y*/
    R_L = calculateR_L(val_x, val_y,channelRedudancy_mode);
    samples_out[2] = R_L[0];
    samples_out[3] = R_L[1];

    //outfile2 << R_L[0] << " " << R_L[1] << endl;

    val2_x = decoder.decode();
    prev2_x = val2_x;
    val1_x = val2_x + prev1_x;
    prev1_x = val1_x;
    val_x = val1_x + preval_x;
    preval_x = val_x;

    val2_y = decoder.decode();
    prev2_y = val2_y;
    val1_y = val2_y + prev1_y;
    prev1_y = val1_y;
    val_y = val1_y + preval_y;
    preval_y = val_y;

    /* calculate the value of each channel from the received values x and y*/
    R_L = calculateR_L(val_x, val_y,channelRedudancy_mode);
    samples_out[4] = R_L[0];
    samples_out[5] = R_L[1];

    //outfile2 << R_L[0] << " " << R_L[1] << endl;

    int n_sample = 6;
    int n = 0;
    int frame_block = 0;
    int flag = 0;
    sf_count_t j;

    unsigned short val3_mod_x, val3_mod_y;
    nBlock = 0;
    sum = 0;
    m = 500;

    while(n < sndFileIn.frames() - 3) {
        val3_x = decoder.decode();
        val2_x = val3_x + prev2_x;
        prev2_x = val2_x;
        val1_x = val2_x + prev1_x;
        prev1_x = val1_x;
        val_x = val1_x + preval_x;
        preval_x = val_x;

        val3_y = decoder.decode();
        val2_y = val3_y + prev2_y;
        prev2_y = val2_y;
        val1_y = val2_y + prev1_y;
        prev1_y = val1_y;
        val_y = val1_y + preval_y;
        preval_y = val_y;

        /* calculate the value of each channel from the received values x and y*/
        R_L = calculateR_L(val_x, val_y, channelRedudancy_mode);
        samples_out[n_sample] = R_L[0];
        samples_out[n_sample + 1] = R_L[1];

        //outfile2 << R_L[0] << " " << R_L[1] << endl;

        nBlock++;
        /* convert samples values to a positive range in order to perform the M estimation */
        val3_mod_x = val3_x >0?  2*val3_x -1 : -2*val3_x;
        val3_mod_y = val3_y >0?  2*val3_y -1 : -2*val3_y;

        sum = sum + val3_mod_x + val3_mod_y;
        if(parameterEstimation_mode == "Block") {
            if (nBlock == blockSize) {
                m = estimateM_fromBlock(sum, blockSize);
                decoder.update(m);

                /*cout << "sum: " << sum << endl;
                cout << "descodificador m updated to " << m << endl;*/

                sum = 0;
                nBlock = 0;
            }
        }
        n_sample = n_sample + 2;
        if(n_sample >= FRAMES_BUFFER_SIZE * sndFileIn.channels()) {
            j = sndFileOut.writef(samples_out.data(), FRAMES_BUFFER_SIZE);
            n_sample = 0;
            frame_block++;
        } else if(frame_block == (nFrames-1) && n_sample == lastFrameSize) {
            j = sndFileOut.writef(samples_out.data(), lastFrameSize);
        }
        n++;
    }
    decoder.close();
    //outfile2.close();

    /* ORIGINAL AUDIO ENTROPY */
    calculateAudioHist("hist_original.txt", argv[1]);
    vector<float> ent = audioEntropy("hist_original.txt");
    cout << "Right channel entropy: " << ent[0] << endl;
    cout << "Left channel entropy: " << ent[1] << endl;
    cout << "Mono channel entropy: " << ent[2] << endl;


    calculateAudioHist("hist_decoded.txt", argv[2]);
    vector<float> ent_dec = audioEntropy("hist_decoded.txt");
    cout << "Right channel entropy: " << ent_dec[0] << endl;
    cout << "Left channel entropy: " << ent_dec[1] << endl;
    cout << "Mono channel entropy: " << ent_dec[2] << endl;

    vector<float> res3_ent = residualsEntropy("hist_res_3.txt");
    cout << "Residuals entropy first channel: " << res3_ent[0] << endl;
    cout << "Residuals entropy second channel: " << res3_ent[1] << endl;

    return 0;
}
void calculateAudioHist(string histFileName, string audioFileName) {
    vector<short> samples(FRAMES_BUFFER_SIZE * 2);

    /* vectors to store the element count of each channel and the mono version */
    vector<long unsigned> hist_r((int)pow(2,16)), hist_l((int)pow(2,16)), hist_mono((int)pow(2,16));

    /* file for storing calculated frequencies */
    ofstream outfile(histFileName);

    SndfileHandle sndFileIn { audioFileName };
    if(sndFileIn.error()) {
        cerr << "Error: invalid input file" << endl;
    }

    if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format" << endl;
    }

    if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format" << endl;
    }

    /* reads all frame from source audio file and computes for each channel the element count */
    while (sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        for(long unsigned int i=0; i< samples.size(); i=i+2) {
            /* NOTE: The addition of the 32767 constant is only needed to convert the negative
             *       samples into positive indexes that can be accessed in the hist vectors to
             *       count each element occurrence */
            hist_r[samples[i]+32767] += 1;
            hist_l[samples[i+1]+32767] += 1;
            hist_mono[((samples[i] + samples[i+1])/2) + 32767] += 1;
        }
    }
    /* write the computed element count */
    for (long unsigned int k=0; k< hist_r.size(); k++) {
        outfile << hist_r [k] << " " << hist_l[k] << " " << hist_mono[k] << endl;
    }
    outfile.close();
}
vector<float> audioEntropy(string fileName) {
    // nº de bits médio/simbolo
    float Pi_r, Pi_l, Pi_mono;
    float ent_r = 0, ent_l = 0, ent_mono = 0;
    unsigned long int count_r = 0, count_l = 0, count_mono = 0;
    vector<float> ent(3);

    vector<unsigned int> hist_r(FRAMES_BUFFER_SIZE), hist_l(FRAMES_BUFFER_SIZE), hist_mono(FRAMES_BUFFER_SIZE);

    ifstream hist_file(fileName);

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

    for (i=0;i<FRAMES_BUFFER_SIZE;i++)
    {
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

vector<long signed> calculateR_L (signed long val_x, signed long val_y, string channelRedudancy_mode) {
    vector<signed long> R_L (2);
    signed long val_l, val_r;

    if(channelRedudancy_mode == "Mid-side") {
        val_l = (2*val_x + val_y)/2;
        val_r = val_l - val_y;
    } else if(channelRedudancy_mode == "Right-side") {
        val_r = val_x;
        val_l = val_r + val_y;
    } else if(channelRedudancy_mode == "Left-side"){
        val_l = val_x;
        val_r = val_l - val_y;
    } else {
        /* Default is Independent Mode*/
        val_r = val_x;
        val_l = val_y;
    }
    R_L[0] = val_r;
    R_L[1] = val_l;
    return R_L;
}
vector<long signed> calculateX_Y (signed long val_r, signed long val_l, string channelRedudancy_mode) {
    vector<signed long> X_Y (2);
    signed long val_x, val_y;

    if(channelRedudancy_mode == "Mid-side") {
        val_x = (val_r + val_l) / 2;    // code (L+R)/2
        val_y = (val_l - val_r);        // code L-R
    } else if(channelRedudancy_mode == "Right-side") {
        val_x = val_r;                  // code R
        val_y = (val_l - val_r);        // code L-R
    } else if(channelRedudancy_mode == "Left-side"){
        val_x = val_l;                  // code L
        val_y = (val_l - val_r);        // code L-R
    } else {
        /* Default is Independent Mode*/
        val_x = val_r;
        val_y = val_l;
    }
    X_Y[0] = val_x;
    X_Y[1] = val_y;
    return X_Y;
}

unsigned int estimateM_fromBlock(unsigned int sum, unsigned int blockSize){
    double mean, alfa;
    unsigned int m;

    mean = (sum/(double)(blockSize*2));
    alfa = mean/(1+mean);
    m = ceil(-1/log2(alfa));

    return m;
}
unsigned int estimateM_fromAllSamples(string audioFileName) {
    SndfileHandle sndFileIn { audioFileName };
    if(sndFileIn.error()) {
        cerr << "Error: invalid input file" << endl;
    }

    if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format" << endl;
    }

    if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format" << endl;
    }


    /* vectors to store the samples from each channel and the calculated mono version */
    vector<short> samples(FRAMES_BUFFER_SIZE * 2);

    vector<short> r_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_1(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_2(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_3(FRAMES_BUFFER_SIZE * 2);

    vector<signed long> X_Y(2);
    unsigned short res_3_mod_x=0, res_3_mod_y=0;
    sf_count_t c;
    unsigned int nFrames = 0;
    unsigned long sum=0;
    unsigned int initial_m;

    /* identifies the first frame of data */
    unsigned int frame = 0;

    while (c=(sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE))) {
        nFrames++;
        for(int i=0; i< c*2; i=i+2) {
            X_Y = calculateX_Y (samples[i], samples[i+1], channelRedudancy_mode);
            if(frame == 0) {
                if (i == 0) {
                    res_0[i] = X_Y[0];
                    res_0[i + 1] = X_Y[1];
                } else if (i == 2) {
                    res_0[i] = X_Y[0];
                    res_1[i] = res_0[i] - res_0[i - 2];

                    res_0[i + 1] = X_Y[1];
                    res_1[i + 1] = res_0[i + 1] - res_0[i - 1];

                } else if (i == 4) {

                    res_0[i] = X_Y[0];
                    res_1[i] = res_0[i] - res_0[i - 2];
                    res_2[i] = res_1[i] - res_1[i - 2];

                    res_0[i + 1] = X_Y[1];
                    res_1[i + 1] = res_0[i + 1] - res_0[i - 1];
                    res_2[i + 1] = res_1[i + 1] - res_1[i - 1];
                    frame = 1;

                }
            } else if (i == 0) {

                res_0[i] = X_Y[0];
                res_1[i] = res_0[i] - res_0[samples.size() - 2];
                res_2[i] = res_1[i] - res_1[samples.size() - 2];
                res_3[i] = res_2[i] - res_2[samples.size() - 2];


                res_0[i + 1] = X_Y[1];
                res_1[i + 1] = res_0[i + 1] - res_0[samples.size() - 1];
                res_2[i + 1] = res_1[i + 1] - res_1[samples.size() - 1];
                res_3[i + 1] = res_2[i + 1] - res_2[samples.size() - 1];

            } else {
                res_0[i] = X_Y[0];
                res_1[i] = res_0[i] - res_0[i-2];
                res_2[i] = res_1[i] - res_1[i-2];
                res_3[i] = res_2[i] - res_2[i-2];

                res_0[i+1] = X_Y[1];
                res_1[i+1] = res_0[i+1] - res_0[i-1];
                res_2[i+1] = res_1[i+1] - res_1[i-1];
                res_3[i+1] = res_2[i+1] - res_2[i-1];
            }
            if((i>4 && frame == 1) || (nFrames>1)) {
                /* convert samples values to a positive range in order to perform the M estimation */
                res_3_mod_x = res_3[i]>0?  2*res_3[i] -1   : -2*res_3[i];
                res_3_mod_y = res_3[i+1]>0?  2*res_3[i+1] -1   : -2*res_3[i+1];
                sum = sum + res_3_mod_x + res_3_mod_y;
            }
        }
    }
    initial_m = estimateM_fromBlock(sum, sndFileIn.frames());
    cout << "sum: " << sum << endl;
    cout << "initial_m: " << initial_m << endl;

    return initial_m;
}
void calculateResHist(string fileName, short res_x, short res_y, unsigned int totalFrames) {
    /* vector to store the element count of the histogram */
    static vector<long unsigned> hist_res_x((int)pow(2,16)), hist_res_y((int)pow(2,16));
    /* count the number of residuals that have been written */
    static unsigned int nSample=0;
    if(nSample==0) {
        cout << "Started Residuals histogram computation" << endl;
    }
    nSample++;
    /* file for writing the residuals histograms for each predictor order */
    ofstream outfile(fileName);
    hist_res_x[res_x+32767] += 1;
    hist_res_y[res_y+32767] += 1;

    if(nSample == totalFrames) {
        cout << "Residuals histogram computation done. Writting destination file..." << endl;
        for(unsigned int k=0; k < hist_res_x.size(); k++) {
            outfile << hist_res_x[k] << " " << hist_res_y[k] << endl;
        }
    }
}
vector<float> residualsEntropy(string fileName) {
    float Pi_x, Pi_y;
    float ent_x = 0, ent_y = 0;
    unsigned long int count_x = 0, count_y = 0;

    vector<unsigned int> hist_res_x(pow(2,16)), hist_res_y(pow(2,16));
    vector<float> ent(2);
    ifstream hist_file(fileName);

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