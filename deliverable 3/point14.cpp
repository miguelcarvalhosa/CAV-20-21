
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

vector<long signed> calculateX_Y (signed long val_r, signed long val_l, string channelRedudancy_mode);
vector<long signed> calculateR_L (signed long val_x, signed long val_y, string channelRedudancy_mode);

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

/* The coded has four different modes of operation depending on the type of channel redudancy:
 * "Mid-side", "Right-side", "Left-side" & "Independent" (default) */
string channelRedudancy_mode = "Left-side";

int main(int argc, char *argv[]) {
    /* vectors to store the samples from each channel and the calculated mono version */
    vector<short> samples(FRAMES_BUFFER_SIZE * 2);

    vector<short> r_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_1(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_2(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_3(FRAMES_BUFFER_SIZE * 2);


    ofstream outfile1("original.txt");
    ofstream outfile2("decoded.txt");
    /* identifies the first frame of data */
    int frame = 0;

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

    GolombEncoder encoder(500,"Residuals.bin");
    int nFrames = 0;
    sf_count_t c;
    unsigned int lastFrameSize;

    vector<signed long> X_Y(2);
    /* reads all frame from source audio file and computes for each channel the element count */
    while (c=sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        nFrames++;
        if(c!=FRAMES_BUFFER_SIZE) {
            lastFrameSize = c;
        }
        for(long unsigned int i=0; i< c*2; i=i+2) {

            X_Y = calculateX_Y (samples[i], samples[i+1], channelRedudancy_mode);
            outfile1 << samples[i] << " " << samples[i+1] << endl;
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

        }
    }
    outfile1.close();
    cout << "nFrame: " << nFrames << endl;
    encoder.close();
    GolombDecoder decoder(500,"Residuals.bin");

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
    outfile2 << R_L[0] << " " << R_L[1] << endl;

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
    outfile2 << R_L[0] << " " << R_L[1] << endl;

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
    outfile2 << R_L[0] << " " << R_L[1] << endl;

    int n_sample = 6;
    int n = 0;
    int frame_block = 0;
    int flag = 0;
    sf_count_t j;

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
        n_sample = n_sample + 2;
        outfile2 << R_L[0] << " " << R_L[1] << endl;

        if(n_sample >= FRAMES_BUFFER_SIZE * sndFileIn.channels()) {
            j = sndFileOut.writef(samples_out.data(), FRAMES_BUFFER_SIZE);
            n_sample = 0;
            frame_block++;
        } else if(frame_block == (nFrames-1) && n_sample == lastFrameSize) {
            j = sndFileOut.writef(samples_out.data(), lastFrameSize);
        }
        n++;
    }
    cout << "n: " << n << endl;
    decoder.close();
    outfile2.close();

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