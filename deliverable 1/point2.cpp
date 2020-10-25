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

    /* Opening video file */
    ifstream myfile (argv[1]);

    /* Processing header */
    getline (myfile,line);

    /* Processing header */
    getline (myfile,line);
    cout << line << endl;
    cout << line.substr(line.find(" W") + 2, line.find(" H") - line.find(" W") - 2) << endl;
    yCols = stoi(line.substr(line.find(" W") + 2, line.find(" H") - line.find(" W") - 2));
    yRows = stoi(line.substr(line.find(" H") + 2, line.find(" F") - line.find(" H") - 2));
    cout << yCols << ", " << yRows << endl;

    return 0;
}