/**
 * \file point2.cpp
 *
 * \brief Contains the resolution of point 2 of Deliverable 1.
 *        This program copies a 4:4:4 video file pixel by pixel.
 *        To use the program, the user must pass two arguments: the input file path and the output file path
 *
 *        Usage: point2 <input file> <output file>
 *
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <fstream>

using namespace cv;
using namespace std;


int main(int argc, char *argv[])
{
    string line; // store the header
    int yCols, yRows; /* frame dimension */
    int end = 0;

    /* Opening input and output video file */
    ifstream infile (argv[1]);
    ofstream outfile (argv[2]);

    /* Processing header */
    getline (infile,line);
    cout << line;
    cout << line.substr(line.find(" W") + 2, line.find(" H") - line.find(" W") - 2) << endl;
    yCols = stoi(line.substr(line.find(" W") + 2, line.find(" H") - line.find(" W") - 2));
    yRows = stoi(line.substr(line.find(" H") + 2, line.find(" F") - line.find(" H") - 2));
    cout << yCols << ", " << yRows << endl;

    /* write line header in dest file*/
    outfile << line << endl;

    /* buffer to store the frame in packet mode */
    unsigned char *frameData = new unsigned char[yCols * yRows * 3];

    while(!end) {

        getline (infile,line); // Skipping word FRAME
        /* load one frame into buffer frameData */
        infile.read((char *)frameData, yCols * yRows * 3);
        outfile << line << endl;

        /* stop process when last frame is detected */
        if(infile.eof())
        {
            cout << "-----------------------------------------------------------" << endl;
            cout << "INPUT FILE " << argv[1] << " COPIED TO " << argv[2] <<endl;
            cout << "TRY RUNNING $ ffplay <argv[1]>.y4m" << endl;
            cout << "            $ ffplay <argv[2]>.y4m" << endl;
            waitKey();
            outfile.close();
            infile.close();
            end = 1;
            break;
        }
        outfile << frameData << endl; //or outfile.write(reinterpret_cast<const char*>(frameData), yCols * yRows * 3);
    }
    return 0;
}

