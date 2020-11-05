/**
 * \file point4.cpp
 *
 * \brief Contains the resolution of point 4 of Deliverable 1.
 *        This program reads a .wav audio file and computes the histogram for each
 *        possible sample value [-2^(15) .. 2^(15)-1].
 *
 *        To use the program, the user must pass two arguments: the input audio file
 *        path and the output histogram count file path
 *
 *        Usage: point5 <input audio file> <output histogram file>
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
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {
    /* vectors to store the samples from each channel and the calculated mono version */
    vector<short unsigned> buf_r(FRAMES_BUFFER_SIZE), buf_l(FRAMES_BUFFER_SIZE), buf_mono(FRAMES_BUFFER_SIZE);
    /* vectors to store the element count of each channel and the mono version */
    vector<long unsigned> hist_r((int)pow(2,16)), hist_l((int)pow(2,16)), hist_mono((int)pow(2,16));

    vector<short> samples(FRAMES_BUFFER_SIZE * 2);

    /* file for storing calculated frequencies */
    ofstream outfile(argv[argc-1]);


    if(argc < 3) {
        cerr << "Usage: wavcp <input file.wav> <output hist.txt>" << endl;
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

    /* reads all frame from source audio file and computes for each channel the element count */
    while (sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        for(long unsigned int i=0; i< samples.size(); i=i+2) {
            /* NOTE: The addition of the 32767 constant is only needed to convert the negative
             *       samples into positive indexes that can be accessed in the hist vectors to
             *       count each element occurrence */
            buf_r[i/2] = samples[i]+32767;
            hist_r[buf_r[i/2]] += 1;
            buf_l[i/2]= samples[i+1]+32767;
            hist_l[buf_l[i/2]] += 1;
            buf_mono[i/2] = ((buf_r[i/2] + buf_l[i/2])/2);
            hist_mono[buf_mono[i/2]] += 1;
            cout << (i/2) << " " << hist_r[i/2] << " " << hist_l[i/2] << " " << hist_mono[i/2] << endl;
        }
    }
    /* write the computed element count */
    for (long unsigned int k=0; k< hist_r.size(); k++) {
        outfile << hist_r [k] << " " << hist_l[k] << " " << hist_mono[k] << endl;
    }
    return 0;
}