#include "GolombDecoder.h"
#include "math.h"
#include <bitset>
using namespace  std;

GolombDecoder::GolombDecoder(unsigned int m, std::string fileName) {
    //verifies if the m values is positive
    if(m < 10000000){
        this->m = m;
        b = floor(log2(m));
    }
    else {
        std::cerr << "The m parameter inserted is not valid." << std::endl;
        std::invalid_argument("The m parameter inserted is not valid");
    }
    /* open the file to be used by bitstream */
    this->fileName = fileName;
    bsr.setFileName(fileName);
};

GolombDecoder::~GolombDecoder() {
}
void GolombDecoder::update(unsigned int m) {
    if(m < 10000000){
        this->m = m;
        b = floor(log2(m));
        //cout << "Number of bits of remainder: " << b << endl;
    }
    else {
        std::cerr << "The m parameter inserted is not valid." << std::endl;
        std::invalid_argument("The m parameter inserted is not valid");
    }
}
std::string GolombDecoder::decodeHeader(unsigned int nChars) {
    std::string header;
    bsr.readString(header, nChars);

    return header;
}
signed int GolombDecoder::decode() {
    unsigned long int q=0, r;
    signed int n;
    signed char signal;

    unsigned long int nBytes_q_r;
    /* variable to store the number of empty bits in the last byte */
    unsigned long nBits_0;

    /* check if coded value is positive or negative */
    if(bsr.readBit()==0) {
        signal = 1;
    } else {
        signal = -1;
    }
    unsigned char bit = bsr.readBit();
    /* count number of bits equal to 1 until terminator appears */
    while(bit!=0){
        q++;
        bit=bsr.readBit();
    }
    /* variable used to store number of unused b+1-bit combinations*/
    unsigned int u = (unsigned int)pow(2,b+1) - m;
    /* variable to store the read value of the first b bits of the remainder */
    unsigned long int bBits;
    /* variable to store the read value of the additional bit */
    unsigned char additionalBit;

    /* read first b bits of the remainder */
    bBits=bsr.readNBits(b);

    /* if the value read from first b bits is less than the computed u, then encode the
     * the remainder in b bits (using its binary representation). If not encode the value
     * resulting from the sum r and u in b+1 bits (using its binary representation) */
    if(bBits<u) {
        r = bBits;
    } else {
        additionalBit = bsr.readBit();
        r = ((bBits << 1) | additionalBit) - u;

    }

    /* calculate the coded value and restore its signal */
    n = (signed int) signal * (q*m+r);
    return n;
}

void GolombDecoder::close(){
    bsr.close();
}