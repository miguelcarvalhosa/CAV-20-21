/**
 * \file point5.cpp
 *
 * \brief Contains the resolution of point 5 of Deliverable 1.
 *        This program reads a 4:4:4 video file and computes, for each frame, the histogram
 *        of three color planes [B,G,R] as well as its grayscale version and its corresponding
 *        entropy.
 *
 *        To use the program, the user must pass two arguments: the input video file
 *        path and the output histogram count file path
 *
 *        Usage: point5 <input video file> <output histogram file>
 *
 *        At the end the program prints, for each frame, the entropy of three color channels as well as
 *        its grayscale version
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */

#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

/*
 * Function Prototypes
 */
float entropy(Mat hist);

int main(int argc, char *argv[])
{
    string header; // store the header
    int yCols, yRows; /* frame dimension */
    int end = 0;
    int y, u, v, r, g, b;
    unsigned char *frameData;

    /* set number of bins and range of values*/
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    /* set the bins to have the same size (uniform) and to clear previous existent histograms (accumulate) */
    bool uniform = true, accumulate = false;
    /* output histogram variables */
    Mat b_hist, g_hist, r_hist, gray_hist;
    int nImages = 1;
    /* list of dims channels */
    int channels = 0; // there is only one channel, the intensity
    float b_ent, g_ent, r_ent, gray_ent;

    /* Opening input and output video file */
    ifstream infile (argv[1]);
    ofstream outfile(argv[2]);

    /* Processing header */
    getline (infile,header);
    cout << header;
    cout << header.substr(header.find(" W") + 2, header.find(" H") - header.find(" W") - 2) << endl;
    yCols = stoi(header.substr(header.find(" W") + 2, header.find(" H") - header.find(" W") - 2));
    yRows = stoi(header.substr(header.find(" H") + 2, header.find(" F") - header.find(" H") - 2));
    cout << yCols << ", " << yRows << endl;

    vector<Mat> bgr_planes;
    /* load image into Mat structure and converts it to BGR */
    Mat color_img = Mat(Size(yCols, yRows), CV_8UC3);
    Mat gray_img = Mat(Size(yCols, yRows), CV_8UC1);
    /* buffer to store the frame in packet mode */
    frameData = new unsigned char[yCols * yRows * 3];

    /* pointer to Mat structure data */
    uchar *buffer;
    int count=0;


    cout << "-------------------------- ENTROPY -----------------------" << endl;

    while(!end) {
        count++;
        getline (infile,header); // Skipping word FRAME
        /* load one frame into buffer frameData */
        infile.read((char *)frameData, yCols * yRows * 3);
        /* stop process when last frame is detected */
        if(infile.eof())
        {
            cout << "-----------------------------------------------------------" << endl;
            infile.close();
            outfile.close();
            end = 1;
            break;
        }
        /* The video is stored in YUV planar mode but OpenCv uses packed modes*/
        buffer = (uchar*)color_img.ptr();
        for(int i = 0 ; i < yRows * yCols * 3 ; i += 3)
        {
            /* Accessing to planar info */
            y = frameData[i / 3];
            u = frameData[(i / 3) + (yRows * yCols)];
            v = frameData[(i / 3) + (yRows * yCols) * 2];

            /* convert to RGB */
            b = (int)(1.164*(y - 16) + 2.018*(u-128));
            g = (int)(1.164*(y - 16) - 0.813*(u-128) - 0.391*(v-128));
            r = (int)(1.164*(y - 16) + 1.596*(v-128));

            /* clipping to [0 ... 255] */
            if(r < 0) r = 0;
            if(g < 0) g = 0;
            if(b < 0) b = 0;
            if(r > 255) r = 255;
            if(g > 255) g = 255;
            if(b > 255) b = 255;

            /* Fill the OpenCV buffer - packed mode: BGRBGR...BGR */
            buffer[i] = b;
            buffer[i + 1] = g;
            buffer[i + 2] = r;
        }

        /* separate B,G,R planes */
        split(color_img, bgr_planes );

        /* convert color image to grayscale*/
        cvtColor(color_img, gray_img, COLOR_RGB2GRAY);
        /* calculate histogram for each plane: BGR */
        calcHist( &bgr_planes[0], nImages, &channels, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
        calcHist( &bgr_planes[1], nImages, &channels, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
        calcHist( &bgr_planes[2], nImages, &channels, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

        /* calculate histogram for the gray scale version */
        calcHist( &gray_img, nImages, &channels, Mat(), gray_hist, 1, &histSize, &histRange, uniform, accumulate );

        /* write the calculated element count for each bin of each histogram in the output file specified */
        for (int k=0; k<histSize; k++) {
            outfile << count << " " << b_hist.at<float>(k) << " " << g_hist.at<float>(k) << " " << r_hist.at<float>(k) << " " << gray_hist.at<float>(k) << endl;
        }
        /* calculate entropy */
        b_ent = entropy(b_hist);
        g_ent = entropy(g_hist);
        r_ent = entropy(r_hist);
        gray_ent = entropy(gray_hist);

        cout << "[" << count << "]";
        cout << "(B,G,R) " << "(" << b_ent << ", " << g_ent << ", " << r_ent << ") " << "& (GRAY) " << gray_ent << endl;
    }

    return EXIT_SUCCESS;
}
/**
 * \brief A function to calculate entropy of an image from its Mat histogram.
 *        It is assumed that the Mat histogram only has the element count of
 *        one of the three channels in the RBG image case.
 * \param hist          Mat structure returned by the calcHist() function
 *
 */
float entropy(Mat hist) {
    float Pi, ent = 0;
    int count = 0;

    for (int i=0;i<256;i++) {
        count += hist.at<float>(i);
    }

    for (int i=0;i<256;i++)
    {
        if(hist.at<float>(i) > 0) {
            Pi = (hist.at<float>(i))/count;
            ent -= Pi*log2(Pi);
        }
    }
    return ent;
}