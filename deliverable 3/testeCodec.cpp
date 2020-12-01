#include "AudioCodec.h"


int main(int argc, char *argv[]) {

    AudioCodec my_codec;

    my_codec.compress("sample01.wav", "sample01.cmp", 500, AudioCodec::REDUNDANCY_INDEPENDENT, AudioCodec::NONE);

    my_codec.decompress("sample01.cmp", "sample01_out.wav");

    return 0;
}