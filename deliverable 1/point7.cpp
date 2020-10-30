/**
 * \file point7.cpp
 *
 * \brief Contains the resolution of points 7 and 9 of Deliverable 1.
 *        This program reads a video in .y4m format with YUV_444 color coding and applies a quantization algorithm to
 *        reduce the number of bits used to represent each pixel.
 *        The program plays both the input and reduced videos.
 *        To use the program, the user must pass two arguments: the input file path and the number of bits to use in the
 *        quantizer.
 *
 *        Usage: point7 <input file> <nbits>
 *
 *        At the end, the program prints the signal-to-noise ration (SNR), as well as the maximum per sample absolute
 *        error.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */


#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;


/*
 * Function prototypes
 */
short quantizer(unsigned short nbits, long min, long max, short value);


int main(int argc, char *argv[])
{

    // Validate the number of arguments
    if(argc < 3) {
        cerr << "Usage: point7 <input file> <nbits>" << endl;
        return 1;
    }

    string header; // store the header
    int yCols, yRows; /* frame dimension */
    int end = 0;
    int y, u, v, r, g, b;
    int nbits = stoi(argv[argc-1]);     // Number of bits to represent the reduced pixel

    long energyIn = 0;      // Input file energy value
    long energyNoise = 0;   // Noise energy value
    int maxError = 0;       // Maximum per sample absolute error

    char inputKey = '?'; /* parse the pressed key */

    /* Opening input video file */
    ifstream infile (argv[argc-2]);

    /* Processing header */
    getline (infile,header);
    cout << header;
    cout << header.substr(header.find(" W") + 2, header.find(" H") - header.find(" W") - 2) << endl;
    yCols = stoi(header.substr(header.find(" W") + 2, header.find(" H") - header.find(" W") - 2));
    yRows = stoi(header.substr(header.find(" H") + 2, header.find(" F") - header.find(" H") - 2));
    cout << yCols << ", " << yRows << endl;

    cout << "Number of bits to use: " << nbits << endl;

    vector<Mat> bgr_planes;
    /* load image into Mat structure and converts it to BGR */
    Mat color_img = Mat(Size(yCols, yRows), CV_8UC3);       // Input frame
    Mat color_imgQ = Mat(Size(yCols, yRows), CV_8UC3);      // Quantized frame
    /* buffer to store the frame in packet mode */
    unsigned char *frameData = new unsigned char[yCols * yRows * 3];

    /* pointer to Mat structure data */
    uchar *buffer, *bufferQ;


    while(!end) {

        getline (infile,header); // Skipping word FRAME
        /* load one frame into buffer frameData */
        infile.read((char *)frameData, yCols * yRows * 3);
        /* stop process when last frame is detected */
        if(infile.eof())
        {
            cout << "-----------------------------------------------------------" << endl;
            waitKey();
            infile.close();
            end = 1;
            break;
        }
        /* The video is stored in YUV planar mode but OpenCv uses packed modes*/
        buffer = (uchar*)color_img.ptr();
        bufferQ = (uchar*)color_imgQ.ptr();
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

            // Quantize each component (BGR)
            char bQ = quantizer(nbits, 0, 255, b);
            char gQ = quantizer(nbits, 0, 255, g);
            char rQ = quantizer(nbits, 0, 255, r);

            int pixelIn = sqrt(pow(b, 2)+pow(g, 2)+pow(r, 2));        // Norm of the input pixel
            int pixelQ = sqrt(pow(bQ, 2)+pow(gQ, 2)+pow(rQ, 2));      // Norm of the quantized pixel

            energyIn += pow(pixelIn, 2);            // Accumulate the input signal energy
            int noise = abs(pixelIn - pixelQ);      // Calculate the error between the input and quantized pixel

            if(noise > maxError) {          // Calculate the maximum error
                maxError = noise;
            }

            energyNoise += pow(noise, 2);           // Accumulate the quantized signal energy

            /* Fill the OpenCV input frame buffer - packed mode: BGRBGR...BGR */
            buffer[i] = b;
            buffer[i + 1] = g;
            buffer[i + 2] = r;

            /* Fill the OpenCV quantized frame buffer - packed mode: BGRBGR...BGR */
            bufferQ[i] = bQ;
            bufferQ[i + 1] = gQ;
            bufferQ[i + 2] = rQ;
        }

        /* generate images to display */
        imshow("Original video", color_img);
        imshow("Reduced video", color_imgQ);
        inputKey = waitKey(1.0 / 50 * 1000);
        if ((char)inputKey == 'q') {
            end = 1;
            break;
        }

    }

    // Print the results
    double SNR = 10*log10((double)(energyIn) / (double)(energyNoise));
    cout << "SNR: " << SNR << " dB" << endl;
    cout << "Max per sample absolute error: " << maxError << endl;

    return EXIT_SUCCESS;
}


/**
 * \brief A function to quantize a sample.
 *
 * \param nbits         Number of bits to represent the quantized sample
 * \param min           Mininum value of the input sample range
 * \param max           Maximun value of the input sample range
 * \param value         Sample to be quantized
 *
 * \return The quantized value
 *
 */
short quantizer(unsigned short nbits, long min, long max, short value) {
    short delta = abs(max-min) / pow(2, nbits);     // Delta, the quantizer level interval, is calculated as
                                                         // 'delta = A / 2^b', where A is the input range and b is the
                                                         // number of bits
    return min + floor((value-min)/delta)*delta + delta/2;
}

