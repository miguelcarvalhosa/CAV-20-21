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

    vector<short unsigned> buf_r(FRAMES_BUFFER_SIZE), buf_l(FRAMES_BUFFER_SIZE), buf_mono(FRAMES_BUFFER_SIZE);
    vector<long unsigned> hist_r((int)pow(2,16)), hist_l((int)pow(2,16)), hist_mono((int)pow(2,16));

    vector<short> samples(FRAMES_BUFFER_SIZE * 2);

    /* file for storing calculated frequencies */
    //size_t nFrames;
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

    // reads nFrames from source audio file and calcul
    while (sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        for(long unsigned int i=0; i< samples.size(); i=i+2) {
            buf_r[i/2] = samples[i]+32767;
            hist_r[buf_r[i/2]] += 1;
            buf_l[i/2]= samples[i+1]+32767;
            hist_l[buf_l[i/2]] += 1;
            buf_mono[i/2] = ((buf_r[i/2] + buf_l[i/2])/2);
            hist_mono[buf_mono[i/2]] += 1;
            cout << (i/2) << " " << hist_r[i/2] << " " << hist_l[i/2] << " " << hist_mono[i/2] << endl;
        }
    }

    for (long unsigned int k=0; k< hist_r.size(); k++) {
        outfile << hist_r [k] << " " << hist_l[k] << " " << hist_mono[k] << endl;
    }
    return 0;
}