#include "GolombDecoder.h"
#include "math.h"

using namespace  std;

GolombDecoder::GolombDecoder(unsigned int m) {
    this->m = m;
    b = ceil(log2(m));
};

GolombDecoder::~GolombDecoder() {
    // ?
}

signed int GolombDecoder::decode() {
    unsigned long int q=0, r;
    signed int n;
    signed char signal;

    if(bsr.readBit()==0) {
        signal = 1;
    } else {
        signal = -1;
    }
    uint8_t bit = bsr.readBit();
    while(bit!=0){
        q++;
        bit=bsr.readBit();
    }

    r=bsr.readNBits(b);
    bsr.close();

    n = (signed int) signal * (q*m+r);
    return n;
}
