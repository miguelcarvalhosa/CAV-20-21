
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
    int teste = 0;
    /* identifies the first frame of data */
    int frame = 0;

    /* file for writing residuals values */
    //ofstream outfile(argv[argc-1]);

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

    GolombEncoder encoder(4,"Residuals.txt");

    /* reads all frame from source audio file and computes for each channel the element count */
    while (sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        for(long unsigned int i=0; i< samples.size(); i=i+2) {
            /* NOTE: The addition of the 32767 constant is only needed to convert the negative
             *       samples into positive indexes that can be accessed in the hist vectors to
             *       count each element occurrence */
            buf_r[i/2] = samples[i];      // Retirei passagem para indicies positivos   !!!
            buf_l[i/2]= samples[i+1];
            buf_mono[i/2] = ((buf_r[i/2] + buf_l[i/2])/2);

            if(i == 0 && frame == 0) {
                teste = samples[0];
                res_0[i] = buf_r[i/2];
                res_0[i+1] = buf_l[i/2];
                encoder.encode(res_0[i]);
                encoder.encode(res_0[i+1]);

            }
            else if(i == 2 && frame == 0) {

                res_0[i] = buf_r[i / 2];
                res_1[i] = res_0[i] - res_0[i - 2];
                encoder.encode(res_1[i]);

                res_0[i + 1] = buf_l[i / 2];
                res_1[i + 1] = res_0[i + 1] - res_0[i - 1];
                encoder.encode(res_1[i+1]);
            }
            else if(i == 4 && frame == 0) {

                res_0[i] = buf_r[i/2];
                res_1[i] = res_0[i] - res_0[i-2];
                res_2[i] = res_1[i] - res_1[i-2];
                encoder.encode(res_2[i]);

                res_0[i+1] = buf_l[i/2];
                res_1[i+1] = res_0[i+1] - res_0[i-1];
                res_2[i+1] = res_1[i+1] - res_1[i-1];
                encoder.encode(res_2[i+1]);
                frame = 1;
            }
            else if(i == 0 && frame != 0){

                res_0[i] = buf_r[i/2];
                res_1[i] = res_0[i] - res_0[samples.size()-2];
                res_2[i] = res_1[i] - res_1[samples.size()-2];
                res_3[i] = res_2[i] - res_2[samples.size()-2];
                encoder.encode(res_3[i]);

                res_0[i+1] = buf_l[i/2];
                res_1[i+1] = res_0[i+1] - res_0[samples.size()-1];
                res_2[i+1] = res_1[i+1] - res_1[samples.size()-1];
                res_3[i+1] = res_2[i+1] - res_2[samples.size()-1];
                encoder.encode(res_3[i+1]);
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

        GolombDecoder decoder(4,"Residuals.txt");


    }

    return 0;
}




