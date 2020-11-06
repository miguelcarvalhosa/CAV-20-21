// Function prototype
void VideoPlayer(int argc, char **argv, int uvRows, int uvCols, int format);

int main(int argc, char** argv) {

    //VideoPlayer(argc,argv, 720, 1280, 444);
    //VideoPlayer(argc,argv, 720, 640, 422);
    VideoPlayer(argc,argv, 360, 640, 420);
}