#include <iostream>
#include <vector>
#include <sndfile.hh>
#include <cmath>
#include <fstream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {
    /* set number of bins and range of values*/
    int histSize = pow(2,16);
    float range[] = { 0, 2*32767 };
    const float* histRange = { range };
    /* set the bins to have the same size (uniform) and to clear previous existent histograms (accumulate) */
    bool uniform = true, accumulate = false;
    /* output histogram variables */
    Mat r_hist, l_hist, mono_hist;
    Mat samples_mono, samples_r, samples_l;
    int nImages = 1;
    /* list of dims channels */
    int channels = 0; // there is only one channel, the intensity

    size_t nFrames;
    short buf [FRAMES_BUFFER_SIZE*2];
    short buf_r [FRAMES_BUFFER_SIZE], buf_l[FRAMES_BUFFER_SIZE], buf_mono[FRAMES_BUFFER_SIZE];

    /* file for storing calculated frequencies */
    ofstream outfile(argv[2]);
    if(argc < 2) {
        cerr << "Usage: wavcp <input file> <output file>" << endl;
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


    vector<short> samples(FRAMES_BUFFER_SIZE * sndFileIn.channels());
    nFrames = sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE);

    /* calHist() function does not calculate histogram for signed data. Therefore
     * the range of values between -32767 and 32767 is converted to 0 to 2*32767
     * in order to compute the frequency of each value */
    for(int i=0; i<FRAMES_BUFFER_SIZE*sndFileIn.channels(); i=i+2) {
        buf_r[i] = samples[i]+32767;
        buf_l[i]= samples[i+1]+32767;
        buf_mono[i] = cvRound((buf_r[i] + buf_l[i])/2)+32767;
        cout << i << " " << buf[i] << " " << buf[i+1] << endl;
    }
    /* convert array to Mat in order to calculate the histograms */
    samples_mono= Mat(1,FRAMES_BUFFER_SIZE,CV_16U, buf_mono);
    samples_r = Mat(1,FRAMES_BUFFER_SIZE,CV_16U, buf_r);
    samples_l= Mat(1,FRAMES_BUFFER_SIZE,CV_16U, buf_l);


    /* calculate histogram for each channel */
    calcHist( &samples_r, nImages, &channels, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );
    calcHist( &samples_l, nImages, &channels, Mat(), l_hist, 1, &histSize, &histRange, uniform, accumulate );
    calcHist( &samples_mono, nImages, &channels, Mat(), mono_hist, 1, &histSize, &histRange, uniform, accumulate );
    for (int k=0; k<FRAMES_BUFFER_SIZE; k++) {
        outfile << r_hist.at<short>(k) << " " << l_hist.at<short>(k) << " " << mono_hist.at<short>(k) << endl;
    }

    return 0;
}
