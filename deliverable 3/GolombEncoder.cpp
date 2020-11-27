#include "GolombEncoder.h"
#include "math.h"
#include <stdint.h>
#include <cstdlib>

using namespace std;

GolombEncoder::GolombEncoder(unsigned int m, std::string fileName) {

    //verifies if the m values is positive
    if(m < 10000000){
        this->m = m;
        b = floor(log2(m));
        cout << "Number of bits of remainder: " << b << endl;
    }
    else {
        std::cerr << "The m parameter inserted is not valid." << std::endl;
        std::invalid_argument("The m parameter inserted is not valid");
    }
    /* open the file to be used by bitstream */
    this->fileName = fileName;
    bsw.setFileName(fileName);
};


GolombEncoder::~GolombEncoder() {
}

void GolombEncoder::update(unsigned int m) {
    if(m < 10000000){
        this->m = m;
        b = floor(log2(m));
        cout << "Number of bits of remainder: " << b << endl;
    }
    else {
        std::cerr << "The m parameter inserted is not valid." << std::endl;
        std::invalid_argument("The m parameter inserted is not valid");
    }
}
void GolombEncoder::encode(signed int value) {
    unsigned long int q, r;
    unsigned int val;

    unsigned long int i;
    /* variable to store total number of bytes uses to represent both quotient and
    * remainder */
    unsigned long int nBytes_q_r;
    /* variable to store the number of empty bits in the last byte */
    unsigned long nBits_0;

    unsigned int u;

    /* add bit with the sign information at the start of each sequence */
    if(value < 0) {
        val = abs(value);
        bsw.writeBit(1);
    } else {
        val = value;
        bsw.writeBit(0);
    }
    /* calculate quotient and remainder values */
    q =  val/m;
    r = val%m;
    u = (unsigned int)pow(2,b+1) - m;

    /* write quotient value in unary code */
    i = q;
    while(i>0) {
        bsw.writeBit(1);
        i--;
    }
    /* write sequence terminator */
    bsw.writeBit(0);

    /* write remainder using truncated binary encoding */
    if(r<u) {
        bsw.writeNBits((uint64_t)r,(uint8_t)b);

        nBytes_q_r = (unsigned long)ceil((float)(1+q+1+b)/8);
        nBits_0 = (unsigned long)(nBytes_q_r*8-(q+2+b));
    } else {
        bsw.writeNBits((uint64_t)(r+u),(uint8_t)(b+1));

        nBytes_q_r = (unsigned long)ceil((float)(1+q+1+ceil(log2(m)))/8);
        nBits_0 = (unsigned long)(nBytes_q_r*8-(q+2+b+1));
    }
    /* fill empty spaces in last byte with zeros */
    cout << "nBits_0: " << nBits_0 << endl;
    if(nBits_0!= 0) {
        bsw.writeNBits(0,nBits_0);
    }

    cout << "Quotient    : " << q << endl;
    cout << "Remainder   : " << r << endl;
    cout << "M parameter : " << m << endl;
}

void GolombEncoder::close(){
    bsw.close();
}