
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
#include <math.h>
#include <fstream>
#include "GolombEncoder.h"
#include "GolombDecoder.h"

using namespace std;


constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {
    /* vectors to store the samples from each channel and the calculated mono version */
    vector<short> buf_r(FRAMES_BUFFER_SIZE), buf_l(FRAMES_BUFFER_SIZE), buf_mono(FRAMES_BUFFER_SIZE);   // TIREI UNSIGNED Q ESTAVA !!!

    vector<short> samples(FRAMES_BUFFER_SIZE * 2);

    vector<short> r_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_1(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_2(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_3(FRAMES_BUFFER_SIZE * 2);

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

    GolombEncoder encoder(1024,"Residuals.bin");
    int expected1, expected2;
    /* reads all frame from source audio file and computes for each channel the element count */
    while (sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        for(long unsigned int i=0; i< samples.size(); i=i+2) {
            /* NOTE: The addition of the 32767 constant is only needed to convert the negative
             *       samples into positive indexes that can be accessed in the hist vectors to
             *       count each element occurrence */
            buf_r[i/2] = samples[i];      // Retirei passagem para indicies positivos   !!!
            buf_l[i/2]= samples[i+1];
            buf_mono[i/2] = ((buf_r[i/2] + buf_l[i/2])/2);
            if(frame == 0) {
                if (i == 0) {

                    res_0[i] = buf_r[i / 2];
                    res_0[i + 1] = buf_l[i / 2];
                    encoder.encode(res_0[i]);
                    encoder.encode(res_0[i + 1]);
                    expected1 = res_0[i];
                    expected2 = res_0[i + 1];
                } else if (i == 2) {

                    res_0[i] = buf_r[i / 2];
                    res_1[i] = res_0[i] - res_0[i - 2];
                    encoder.encode(res_1[i]);

                    res_0[i + 1] = buf_l[i / 2];
                    res_1[i + 1] = res_0[i + 1] - res_0[i - 1];
                    encoder.encode(res_1[i + 1]);
                } else if (i == 4) {

                    res_0[i] = buf_r[i / 2];
                    res_1[i] = res_0[i] - res_0[i - 2];
                    res_2[i] = res_1[i] - res_1[i - 2];
                    encoder.encode(res_2[i]);

                    res_0[i + 1] = buf_l[i / 2];
                    res_1[i + 1] = res_0[i + 1] - res_0[i - 1];
                    res_2[i + 1] = res_1[i + 1] - res_1[i - 1];
                    encoder.encode(res_2[i + 1]);
                    frame = 1;
                }
            }
            else if (i == 0) {

                res_0[i] = buf_r[i / 2];
                res_1[i] = res_0[i] - res_0[samples.size() - 2];
                res_2[i] = res_1[i] - res_1[samples.size() - 2];
                res_3[i] = res_2[i] - res_2[samples.size() - 2];
                encoder.encode(res_3[i]);

                res_0[i + 1] = buf_l[i / 2];
                res_1[i + 1] = res_0[i + 1] - res_0[samples.size() - 1];
                res_2[i + 1] = res_1[i + 1] - res_1[samples.size() - 1];
                res_3[i + 1] = res_2[i + 1] - res_2[samples.size() - 1];
                encoder.encode(res_3[i + 1]);
            }
            else {

                res_0[i] = buf_r[i/2];
                res_1[i] = res_0[i] - res_0[i-2];
                res_2[i] = res_1[i] - res_1[i-2];
                res_3[i] = res_2[i] - res_2[i-2];
                encoder.encode(res_3[i]);

                res_0[i+1] = buf_l[i/2];
                res_1[i+1] = res_0[i+1] - res_0[i-1];
                res_2[i+1] = res_1[i+1] - res_1[i-1];
                res_3[i+1] = res_2[i+1] - res_2[i-1];
                encoder.encode(res_3[i+1]);
            }

        }



    }
    encoder.close();
    GolombDecoder decoder(8192,"Residuals.bin");

    signed int val_r, val_l, preval_r,preval_l;
    signed int val1_r, val1_l, prev1_r, prev1_l;
    signed int val2_r, val2_l, prev2_r, prev2_l;
    signed int val3_r, val3_l, prev3_r, prev3_l;
    signed int actual_r, actual_l;
    signed int prev_r, prev_l;

    SndfileHandle sndFileOut { argv[argc-1], SFM_WRITE, sndFileIn.format(),
       sndFileIn.channels(), sndFileIn.samplerate() };

    if(sndFileOut.error()) {
        cerr << "Error: invalid output file" << endl;
        return 1;
    }

    vector<short> samples_out(FRAMES_BUFFER_SIZE * sndFileIn.channels());
    val_r = decoder.decode();
    preval_r = val_r;
    val_l = decoder.decode();
    preval_l = val_l;

    samples_out[0] = val_r;
    samples_out[1] = val_l;

    val1_r = decoder.decode();
    prev1_r = val1_r;
    val_r = val1_r + preval_r;
    preval_r = val_r;
    samples_out[2] = val_r;
    samples_out[3] = val_l;

    val1_l = decoder.decode();
    prev1_l = val1_l;
    val_l = val1_l + preval_l;
    preval_l = val_l;
    samples_out[4] = val_r;
    samples_out[5] = val_l;

    val2_r = decoder.decode();
    prev2_r = val2_r;
    val1_r = val2_r + prev1_r;
    prev1_r = val1_r;
    val_r = val1_r + preval_r;
    preval_r = val_r;
    samples_out[6] = val_r;
    samples_out[7] = val_l;

    val2_l = decoder.decode();
    prev2_l = val2_l;
    val1_l = val2_l + prev2_l;
    prev1_l = val1_l;
    val_l = val1_l + preval_l;
    preval_l = val_l;
    samples_out[8] = val_r;
    samples_out[9] = val_l;
    int n_sample = 10;
    int n;
    while(n < sndFileIn.frames()-2) {

        val3_r = decoder.decode();
        prev3_r = val3_r;
        val2_r = val3_r + prev2_r;
        prev2_r = val2_r;
        val1_r = val2_r + prev1_r;
        prev1_r = val1_r;
        val_r = val1_r + preval_r;
        preval_r = val_r;


        val3_l = decoder.decode();
        prev3_l = val3_l;
        val2_l = val3_l + prev2_l;
        val2_l = decoder.decode();
        prev2_l = val2_l;
        val1_l = val2_l + prev2_l;
        prev1_l = val1_l;
        val_l = val1_l + preval_l;
        preval_l = val_l;

        if(n_sample > FRAMES_BUFFER_SIZE*2 - 2) {

            sndFileOut.writef(samples_out.data(), FRAMES_BUFFER_SIZE);
            n_sample = 0;
        }

        n++;
        n_sample = n_sample + 2;
    }

    decoder.close();
    return 0;
}




