/*! \file main.cpp
 *	\brief Split a RGB image into individual channels (Red, Blue, Green) 
 *	       and add a random color border to the image.
 *	
 *	File main.cpp is a program to split a rgb image into 3 distinct channels (Red, Green and Blue).
 *	This program also add a random color board to the image.
 *  Press any key to move on to the next feature. 
 *	Press ESC to exit.
 *
 */


#include "opencv2/opencv.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

/*! \var RNG rng 
 *  \brief Random Number Generator. 
 *  
 *  Random number generator. It encapsulates the state 
 *  (currently, a 64-bit integer) and has methods to return scalar 
 *  random values and to fill arrays with random values.
 */
RNG rng;

/*! \fn Mat extractRed(Mat img)
    \brief Extract the red component of the image 
           and return the correspondent red image.
 
    \param img Source image. 
*/
Mat extractRed(Mat img)
{	
	// Declare the variables
	vector<Mat> rgbChannels(3); //Mat rgbchannel[3];
	vector<Mat> channels;
	Mat tmp, r;

	split(img, rgbChannels);

	tmp = Mat::zeros(Size(img.cols, img.rows), CV_8UC1);

	channels.push_back(tmp);
	channels.push_back(tmp);
	channels.push_back(rgbChannels[2]);

	merge(channels, r);

	return r;

}


/*! \fn Mat extractGreen(Mat img)
    \brief Extract the green component of the image 
           and return the correspondent green image.
 
    \param img Source image. 
*/
Mat extractGreen(Mat img)
{	
	// Declare the variables
	vector<Mat> rgbChannels(3); //Mat rgbchannel[3];
	vector<Mat> channels;
	Mat tmp,g;

	split(img, rgbChannels);

	tmp = Mat::zeros(Size(img.cols, img.rows), CV_8UC1);

	channels.push_back(tmp);
	channels.push_back(rgbChannels[1]);
	channels.push_back(tmp); 

	merge(channels, g);

	return g;
}

/*! \fn Mat extractBlue(Mat img)
    \brief Extract the blue component of the image 
           and return the correspondent blue image.
 
    \param img Source image. 
*/
Mat extractBlue(Mat img)
{	
	// Declare the variables
	vector<Mat> rgbChannels(3); //Mat rgbchannel[3];
	vector<Mat> channels;
	Mat tmp, b;

	split(img, rgbChannels);

	tmp = Mat::zeros(Size(img.cols, img.rows), CV_8UC1);

	channels.push_back(rgbChannels[0]);
	channels.push_back(tmp);
	channels.push_back(tmp);

	merge(channels, b);

	return b;
}


/*! \fn Mat addBorder(Mat img)
    \brief Constant value border.
    
    Applies a padding of a constant value for the whole border. 
    This value will be updated randomly pressing in any key.
    Press ESC to exit.

    \param img Source image. 
*/
Mat addBorder(Mat img)
{
	// Declare the variables
	Mat dst;
	int top, bottom, left, right;
	int borderType = BORDER_CONSTANT; // BORDER_REPLICATE

	// Initialize arguments for the filter
	top = (int) (0.05*img.rows); 
	bottom = top;
	left = (int) (0.05*img.cols); 
	right = left;

	// Random color
	Scalar value( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
	
	// Form a border around the image
	copyMakeBorder( img, dst, top, bottom, left, right, borderType, value );

	return dst;
}

int main( int argc, char** argv )
{	
	cout << "OpenCV version: " << CV_VERSION << endl;

	Mat image;
	
	// Loads an image
	image = imread("../rgb.jpg", IMREAD_COLOR );
	//image = imread("../ua.png", IMREAD_COLOR );

	// Check if image is loaded fine
	if( image.empty()) 
	{
		printf("Error opening image\n");
		return -1;
	}

	namedWindow("Original", WINDOW_AUTOSIZE);
	imshow("Original", image);

	Mat red;
	red = extractRed(image);
	namedWindow("Red", WINDOW_AUTOSIZE);
	imshow("Red", red);

	Mat green;
	green = extractGreen(image);
	namedWindow("Green",WINDOW_AUTOSIZE);
	imshow("Green", green);

	Mat blue;
	blue = extractBlue(image);
	namedWindow("Blue", WINDOW_AUTOSIZE);
	imshow("Blue", blue);

	char c = (char)waitKey(0);

	if( c == 27 ) // esc
	{
		return 0;
	}

	while(1)
	{
		Mat border_img;
		border_img = addBorder(image);
		
		namedWindow("Example", WINDOW_AUTOSIZE);
		imshow("Example", border_img);
		
		c = (char)waitKey(0);

		if( c == 27 ) // esc
		{
			break;
		}

	}	

	return 0;
}