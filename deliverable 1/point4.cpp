#include <iostream>
#include <vector>
#include <sndfile.hh>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {
    /* set number of bins and range of values*/
    int histSize = 256;
    float range[] = { 0, pow(2,16) };
    const float* histRange = { range };
    /* set the bins to have the same size (uniform) and to clear previous existent histograms (accumulate) */
    bool uniform = true, accumulate = false;
    /* output histogram variables */
    Mat r_hist, l_hist;
    int nImages = 1;
    /* list of dims channels */
    int channels = 0; // there is only one channel, the intensity


    if(argc < 3) {
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

    SndfileHandle sndFileOut { argv[argc-1], SFM_WRITE, sndFileIn.format(),
                               sndFileIn.channels(), sndFileIn.samplerate() };
    if(sndFileOut.error()) {
        cerr << "Error: invalid output file" << endl;
        return 1;
    }

    size_t nFrames;
    vector<short> samples(FRAMES_BUFFER_SIZE * sndFileIn.channels());
    // reads nFrames from source audio file to destination file
    while((nFrames = sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)))
        sndFileOut.writef(samples.data(), nFrames);

    Mat samples_r (1,FRAMES_BUFFER_SIZE ,CV_16UC1, samples[0]);
    Mat samples_l (1,FRAMES_BUFFER_SIZE ,CV_16UC1, samples[1]);
    /* calculate histogram for each plane: BGR */
    calcHist( &samples_r, nImages, &channels, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );
    calcHist( &samples_l, nImages, &channels, Mat(), l_hist, 1, &histSize, &histRange, uniform, accumulate );

    /* generate image to display */
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound( (double) hist_w/histSize );
    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

    /* normalize each histogram between 0 and histImage.rows in order to display it */
    /*    @ dtype = -1: src and dst are the same type (Mat)           */
    /*    @ norm_type = adjust values between the specified range:    */
    normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(l_hist, l_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

    for( int i = 0; i < histSize+1; i++ )
    {
        line( histImage, Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
              Point( bin_w*(i+1), hist_h - cvRound(r_hist.at<float>(i+1)) ),
              Scalar( 255, 0, 0), 2, 8, 0  );

        line( histImage, Point( bin_w*(i), hist_h - cvRound(l_hist.at<float>(i)) ),
              Point( bin_w*(i+1), hist_h - cvRound(l_hist.at<float>(i+1)) ),
              Scalar( 0, 255, 0), 2, 8, 0  );
    }
    imshow("BGR Histogram", histImage );
    waitKey(0);
    return 0;

}

