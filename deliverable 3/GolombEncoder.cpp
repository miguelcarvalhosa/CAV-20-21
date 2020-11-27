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

    } else {
        bsw.writeNBits((uint64_t)(r+u),(uint8_t)(b+1));

    }
    /* fill empty spaces in last byte with zeros */



    cout << "Quotient    : " << q << endl;
    cout << "Remainder   : " << r << endl;
    cout << "M parameter : " << m << endl;
}

void GolombEncoder::close(){
    bsw.close();
}