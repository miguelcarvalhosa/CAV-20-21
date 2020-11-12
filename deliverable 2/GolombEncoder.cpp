#include "GolombEncoder.h"
#include "math.h"
#include <stdint.h>
#include <cstdlib>

using namespace std;

GolombEncoder::GolombEncoder(unsigned int m) {

    this->m = m;
    b = ceil(log2(m));
    cout << "Number of bits of remainder: " << b << endl;
};

GolombEncoder::~GolombEncoder() {
    // ?
}

void GolombEncoder::encode(signed int value) {
    unsigned long int q, r;
    unsigned int val;
    unsigned long int nBits_0;

    unsigned long int i;
    unsigned long int unary = 0;
    unsigned long int nBytes_q_r;
    unsigned long int n64BitSets = 0;
    unsigned int  j;

    if(value < 0) {
        val = abs(value);
        bsw.writeBit(1);
    } else {
        val = value;
        bsw.writeBit(0);
    }
    q =  val/m;
    r = val%m;

    /* write quotient value in unary code */
    i = q;
    while(i>0) {
        unary = unary << 1;
        unary |= 1;
        i--;
    }

    n64BitSets = ceil((float)(q+1+b)/64);
    j = n64BitSets;

    unsigned long var;
    unsigned long  bitsLeft = q-(64*(n64BitSets-1));

    if(q>0 & q <= 64) {
        bsw.writeNBits(unary,q);
    } else {
        while(j>0) {
            if(j>1) {
                var = unary >> (uint8_t) pow(64,j-1);
                bsw.writeNBits(var,64);
            }
            if(j==1 & bitsLeft>0) {
                    var = unary >> 64-(bitsLeft%64);
                    bsw.writeNBits(var,bitsLeft);
            }
            j--;
        }
    }
    // q + 1 bits escritos
    bsw.writeBit(0);
    bsw.writeNBits((uint64_t)r,(uint8_t)b);

    /* write remainder in binary code */
    nBytes_q_r = (unsigned long)ceil((float)(q+1+b)/8);

    /* calculate number of unused bits of last byte and fill it with zeros */
    nBits_0 = (nBytes_q_r*8-(q+1+b));

    if(nBits_0!= 0) {
        bsw.writeNBits(0,nBits_0);
    }
    bsw.close();

    cout << "Quotient    : " << q << endl;
    cout << "Remainder   : " << r << endl;
    cout << "M parameter : " << m << endl;
}