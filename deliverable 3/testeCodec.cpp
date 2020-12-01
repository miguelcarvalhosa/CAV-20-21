#include "AudioCodec.h"


int main(int argc, char *argv[]) {

    AudioCodec codec;

    codec.compress("sample02.wav", "sample02.cmp", 500);

    codec.decompress("sample02.cmp", "sample02_out.wav");

    return 0;
}