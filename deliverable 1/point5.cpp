#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

float entropy(Mat hist) {
    float Pi, ent = 0;
    int count = 0;

    for (int i=0;i<257;i++) {
        count += hist.at<float>(i);
    }

    for (int i=0;i<256;i++)
    {
        if(hist.at<float>(i) > 0) {
            Pi = (hist.at<float>(i))/count;
            //cout << i << " " << Pi << endl;
            ent -= Pi*log2(Pi);
        }
    }
    return ent;
}

int main(int argc, char *argv[])
{
    string header; // store the header
    int yCols, yRows; /* frame dimension */
    int end = 0;
    int y, u, v, r, g, b;

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
    char inputKey = '?'; /* parse the pressed key */

    /* Opening input and output video file */
    ifstream infile (argv[1]);

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
    unsigned char *frameData = new unsigned char[yCols * yRows * 3];

    /* pointer to Mat structure data */
    uchar *buffer;
    int count=0;

    cout << "-------------------------- ENTROPY -----------------------" << endl;
    while(!end) {

        getline (infile,header); // Skipping word FRAME
        /* load one frame into buffer frameData */
        infile.read((char *)frameData, yCols * yRows * 3);
        //uchar (*b)[yCols] = reinterpret_cast<uchar (*)[yCols]>(frameData);
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

        merge(bgr_planes, color_img );
        /* generate image to display */
        int hist_w = 512, hist_h = 400;
        int bin_w = cvRound( (double) hist_w/histSize );
        Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

        /* normalize each histogram between 0 and histImage.rows in order to display it */
        /*    @ dtype = -1: src and dst are the same type (Mat)           */
        /*    @ norm_type = adjust values between the specified range:    */
        normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
        normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
        normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
        normalize(gray_hist, gray_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

        for( int i = 0; i < histSize+1; i++ )
        {
            line( histImage, Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                  Point( bin_w*(i+1), hist_h - cvRound(b_hist.at<float>(i+1)) ),
                  Scalar( 255, 0, 0), 2, 8, 0  );

            line( histImage, Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                  Point( bin_w*(i+1), hist_h - cvRound(g_hist.at<float>(i+1)) ),
                  Scalar( 0, 255, 0), 2, 8, 0  );

            line( histImage, Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                  Point( bin_w*(i+1), hist_h - cvRound(r_hist.at<float>(i+1)) ),
                  Scalar( 0, 0, 255), 2, 8, 0  );

            line( histImage, Point( bin_w*(i), hist_h - cvRound(gray_hist.at<float>(i)) ),
                  Point( bin_w*(i+1), hist_h - cvRound(gray_hist.at<float>(i+1)) ),
                  Scalar( 255, 255, 255), 2, 8, 0  );
            line( histImage, Point( bin_w*(i), hist_h - cvRound(gray_hist.at<float>(i)) ),
                  Point( bin_w*(i+1), hist_h - cvRound(gray_hist.at<float>(i+1)) ),
                  Scalar( 255, 255, 255), 2, 8, 0  );
        }
        imshow("BGR Histogram", histImage );
        //imshow("BGR Histogram", color_img);
        //imshow("BGR Histogram", gray_img);
        inputKey = waitKey(1.0 / 50 * 1000);
        if ((char)inputKey == 'q') {
            end = 1;
            break;
        }

        b_ent = entropy(b_hist);
        g_ent = entropy(g_hist);
        r_ent = entropy(r_hist);
        gray_ent = entropy(gray_hist);

        count++;
        cout << "[" << count << "]";
        cout << "(B,G,R) " << "(" << b_ent << ", " << g_ent << ", " << r_ent << ") " << "& (GRAY) " << gray_ent << endl;
    }
    return EXIT_SUCCESS;
}

