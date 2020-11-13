#include "GolombEncoder.h"
#include "math.h"
#include <stdint.h>
#include <cstdlib>

using namespace std;

GolombEncoder::GolombEncoder(unsigned int m) {

    //Verifies if the m values is positive
    if(m < 10000000){
        this->m = m;
        b = ceil(log2(m));
        cout << "Number of bits of remainder: " << b << endl;
    }
    else {
        std::cerr << "The m parameter inserted is not valid." << std::endl;
        std::invalid_argument("The m parameter inserted is not valid");
    }

};

GolombEncoder::~GolombEncoder() {
}

void GolombEncoder::encode(signed int value) {
    unsigned long int q, r;
    unsigned int val;
    unsigned long var;
    unsigned long int nBits_0;

    unsigned long int i;
    unsigned long int unary = 0;
    unsigned long int nBytes_q_r;
    unsigned long int n64BitSets = 0;
    unsigned int  j;

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

    /* write quotient value in unary code */
    i = q;
    while(i>0) {
        unary = unary << 1;
        unary |= 1;
        i--;
    }
    /* variable to store number of 64bit sets needed to encode both quotient and
     * remainder */
    n64BitSets = ceil((float)(q+1+b)/64);
    j = n64BitSets;

    /* variable to store total number of bytes uses to represent both quotient and
     * remainder*/
    nBytes_q_r = (unsigned long)ceil((float)(q+1+b)/8);

    /* variable to store the number of bits that are empty in last set of 64 bits
     * after quotient is written to the file */
    unsigned long  bitsLeft = q-(64*(n64BitSets-1));


    /* Since the max number of bits that can be written in a single call to the function
     * writeNbits() of BitStream_Write is 64, we need to separate each 64bit frame that
     * composes the remainder and write it separately */

    if(q>0 & q <= 64) {
        /* number of bits occupied by q < 64 -> write the q bit sequence in one call */
        bsw.writeNBits(unary,q);
    } else {
        /* number of bits occupied by q > 64 -> write each 64bit composed frame in j calls
         * to function writeNBits*/
        while(j>0) {
            if(j>1) {
                var = unary >> (uint8_t) pow(64,j-1);
                bsw.writeNBits(var,64);
            }
            /* Last 64 bit frame  composition */
            if(j==1 & bitsLeft>0) {
                    var = unary >> 64-(bitsLeft%64);
                    bsw.writeNBits(var,bitsLeft);
            }
            j--;
        }
    }
    /* Write sequence terminator */
    bsw.writeBit(0);

    /* Write the remainder binary sequence */
    bsw.writeNBits((uint64_t)r,(uint8_t)b);

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