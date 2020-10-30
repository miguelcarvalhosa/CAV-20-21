#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <fstream>

using namespace cv;
using namespace std;


int main(int argc, char** argv)
{
    string line; // store the header
    int yCols, yRows; /* frame dimension */
    int fps = 40; /* frames per second */
    int i, n, r, g, b, y, u, v; /* auxiliary variables */
    unsigned char *imgData; // file data buffer
    unsigned char *imgDataOriginal;
    uchar *buffer; // unsigned char pointer to the Mat data
    char inputKey = '?'; /* parse the pressed key */
    int end = 0, playing = 1, loop = 0; /* control variables */
    int uvRows;
    uvRows = 360;
    int uvCols;
    uvCols = 640;

    int rowInter = 2;
    int colInter = 2;

    /* check for the mandatory arguments */
    if( argc < 2 ) {
        cerr << "Usage: PlayerYUV444 filename" << endl;
        return 1;
    }

    /* Opening video file */
    ifstream myfile (argv[1]);

    /* Processing header */
    getline (myfile,line);
    cout << line << endl;
    cout << line.substr(line.find(" W") + 2, line.find(" H") - line.find(" W") - 2) << endl;
    yCols = stoi(line.substr(line.find(" W") + 2, line.find(" H") - line.find(" W") - 2));
    yRows = stoi(line.substr(line.find(" H") + 2, line.find(" F") - line.find(" H") - 2));
    cout << yCols << ", " << yRows << endl;
    /* Parse other command line arguments */
    for(n = 1 ; n < argc ; n++)
    {
        if(!strcmp("-fps", argv[n]))
        {
            fps = atof(argv[n+1]);
            n++;
        }

        if(!strcmp("-wait", argv[n]))
        {
            playing = 0;
        }

        if(!strcmp("-loop", argv[n]))
        {
            loop = 1;
        }
    }

    /* data structure for the OpenCv image */
    Mat img = Mat(Size(yCols, yRows), CV_8UC3);

    /* buffer to store the frame from file*/

    imgDataOriginal = new unsigned char[yCols * yRows + 2 * (uvRows * uvCols)];

    /* buffer to store the frame in YUV 4:4:4*/
    imgData = new unsigned char[yCols * yRows * 3];

    /* create a window */
    namedWindow("rgb", WINDOW_AUTOSIZE);


    while(!end)
    {
        /* load a new frame, if possible */
        getline (myfile,line); // Skipping word FRAME
        myfile.read((char *)imgDataOriginal, yCols * yRows + 2 * (uvRows * uvCols));
        if(myfile.gcount() == 0)
        {
            if(loop)
            {
                myfile.clear();
                myfile.seekg(0);
                getline (myfile,line); // read the header
                continue;
            }
            else
            {
                end = 1;
                break;
            }
        }

        //4:4:4
        //unsigned char *imageDate = imageDataOriginal;

        //4:2:2 ou 4:2:0 componente y
        memcpy(imgData, imgDataOriginal, yCols * yRows);

         //4:2:0
        for(int r = 0 ; r < uvRows ; r++) {
            for(int c = 0 ; c < uvCols ; c++) {
                for(i = 0 ; i < colInter ; i++) {
                    for(int j = 0 ; j < rowInter ; j++) {
                        imgData[(r * 2 + i) * yCols + (c * 2 + j) + yCols * yRows] =  imgDataOriginal[(r * uvCols + c) + yCols * yRows];
                        imgData[(r * 2 + i) * yCols + (c * 2 + j) + yCols * yRows + yCols * yRows] = imgDataOriginal[(r * uvCols + c) + yCols * yRows + uvCols * uvRows];
                    }
                }
            }
        }
        /* The video is stored in YUV planar mode but OpenCv uses packed modes*/
        buffer = (uchar*)img.ptr();

        for(i = 0 ; i < yRows * yCols * 3 ; i += 3)
        {
            /* Accessing to planar info */
            y = imgData[i / 3];
            u = imgData[(i / 3) + (yRows * yCols)];
            v = imgData[(i / 3) + (yRows * yCols) * 2];

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

            /* if you need the inverse formulas */
            //y = r *  .299 + g *  .587 + b *  .114 ;
            //u = r * -.169 + g * -.332 + b *  .500  + 128.;
            //v = r *  .500 + g * -.419 + b * -.0813 + 128.;

            /* Fill the OpenCV buffer - packed mode: BGRBGR...BGR */
            buffer[i] = b;
            buffer[i + 1] = g;
            buffer[i + 2] = r;
        }


        /* display the image */
        imshow( "rgb", img );

        if(playing)
        {
            /* wait according to the frame rate */
            inputKey = waitKey(1.0 / fps * 1000);

        }
        else
        {
            /* wait until user press a key */
            inputKey = waitKey(0);

        }

        /* parse the pressed keys, if any */
        switch((char)inputKey)
        {
            case 'q':
                end = 1;
                break;

            case 'p':
                playing = playing ? 0 : 1;
                break;
        }
    }
    return 0;
}

//return 0;