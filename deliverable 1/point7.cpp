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
short quantizer(short value, unsigned char nbits);


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
    int y, u, v, r, g, b, rQ, gQ, bQ;
    int nbits = stoi(argv[argc-1]);     // Number of bits to represent the reduced pixel

    vector<long> energyIn(3);      // Input file energy value
    vector<long> energyNoise(3);   // Noise energy value
    vector<int> maxError(3);       // Maximum per sample absolute error

    for(int i=0; i<3; i++) {
        energyIn[i] = 0;
        energyNoise[i] = 0;
        maxError[i] = 0;
    }

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

            // Quantize each component (YUV)
            int yQ = quantizer(y, nbits);
            int uQ = quantizer(u, nbits);
            int vQ = quantizer(v, nbits);

            /* convert to RGB */
            b = (int)(1.164*(y - 16) + 2.018*(u-128));
            g = (int)(1.164*(y - 16) - 0.813*(u-128) - 0.391*(v-128));
            r = (int)(1.164*(y - 16) + 1.596*(v-128));

            bQ = (int)(1.164*(yQ - 16) + 2.018*(uQ-128));
            gQ = (int)(1.164*(yQ - 16) - 0.813*(uQ-128) - 0.391*(vQ-128));
            rQ = (int)(1.164*(yQ - 16) + 1.596*(vQ-128));


            /* clipping to [0 ... 255] */
            if(r < 0) r = 0;
            if(g < 0) g = 0;
            if(b < 0) b = 0;
            if(r > 255) r = 255;
            if(g > 255) g = 255;
            if(b > 255) b = 255;
            if(rQ < 0) rQ = 0;
            if(gQ < 0) gQ = 0;
            if(bQ < 0) bQ = 0;
            if(rQ > 255) rQ = 255;
            if(gQ > 255) gQ = 255;
            if(bQ > 255) bQ = 255;

            energyIn[0] += pow(y, 2);            // Accumulate the input signal energy
            energyIn[1] += pow(u, 2);            // Accumulate the input signal energy
            energyIn[2] += pow(v, 2);            // Accumulate the input signal energy

            vector<int> noise(3);        // Calculate the error between the input and quantized pixel
            noise[0] = abs(y - yQ);
            noise[1] = abs(u - uQ);
            noise[2] = abs(v - vQ);
            for(int i=0; i<3; i++) {
                if (noise[i] > maxError[i]) {          // Calculate the maximum error
                    maxError[i] = noise[i];
                }
                energyNoise[i] += pow(noise[i], 2);           // Accumulate the quantized signal energy
            }

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
    vector<double> SNR(3);
    for(int i=0; i<3; i++) {
        SNR[i] = 10 * log10((double) (energyIn[i]) / (double) (energyNoise[i]));
        cout << "SNR: " << SNR[i] << " dB" << endl;
        cout << "Max per sample absolute error: " << maxError[i] << endl;
    }

    return EXIT_SUCCESS;
}


/**
 * \brief A function to quantize a sample.
 *
 * \param value         Sample to be quantized
 * \param nbits         Number of bits to reduce in the quantized sample
 *
 * \return The quantized value
 *
 */
short quantizer(short value, unsigned char nbits) {
    return ((value >> nbits) << nbits);
}

