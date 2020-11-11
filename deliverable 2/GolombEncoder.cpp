//
// Created by joao on 09/11/20.
//

#include "GolombEncoder.h"
#include "math.h"
#include <stdint.h>
#include <vector>
#include <bitset>
using namespace std;


GolombEncoder::GolombEncoder(uint32_t m) {

    this->m = m;
    b = ceil(log2(m));
    cout << "b: " << b << endl;
};

GolombEncoder::~GolombEncoder() {
    // ?
}

void GolombEncoder::encode(uint64_t val) {
    cout << "val: " <<val << endl;
    uint64_t q, r;
    uint64_t var;
    cout << "m: " <<m << endl;
    q =  val/m;
    cout << "q: " << q << endl;
    r = val%m;

    /* write quotient value in unary code */
    uint64_t i = q;
    uint64_t unary = 0;
    uint64_t nBytes_q, nBytes_q_r;
    while(i>0) {
        unary = unary << 1;
        unary |= 1;
        i--;
    }
    // q + 1 bits escritos
    cout << "r: " << r << endl;

    /* write remainder in binary code */
    nBytes_q = (uint64_t)ceil((float)(q+1)/8);
    nBytes_q_r = (uint64_t)ceil((float)(q+1+b)/8);

    vector <unsigned char> bytes(nBytes_q_r>nBytes_q?nBytes_q_r:nBytes_q,1);
    cout << "byte_size: " << bytes.size() << endl;
    cout << "resq: " << nBytes_q << endl;
    cout << "resr: " << nBytes_q_r << endl;

    bsw.writeNBits(unary,q);
    bsw.writeBit(0);
    bsw.writeNBits(r,b);
    uint64_t v = 0>>(nBytes_q_r*8-(q+1+b));
    uint8_t pos = nBytes_q_r*8-(q+1+b);
    bsw.writeNBits(v,pos);
    /* se um nº int de bits foi lido */
//    if((q+1) + b <= 8) {
//        /*  the remainder need another byte to be stored */
//        var = ((unary << (b+1)) | r) << (8-(b+1+q));
//        cout << "var: " << var << endl;
//        bsw.writeNBits(var,8);
//    /* for the case when the quotient and remainder can both fit in the same number of bytes*/
//    } else if (nBytes_q == nBytes_q_r){
//        uint64_t i=nBytes_q;
//        while(i>0) {
//            //cout << "i: " << i << endl;
//            if(i>2) {
//                /* when the number of bytes needed to store both q and r > 2, we need to perform
//                 * 8^(i-2) shifts to obtain the 8 bits of any byte with index>2 */
//                bytes[i] = unary >> (uint64_t)pow(8,i-2) >> (q % 8);
//                cout << "2: " << bitset<8>(bytes[i]) << endl;
//            }
//            if (i == 2) {
//                bytes[i] = unary >> (q % 8);
//                cout << "3: " << bitset<8>(bytes[i]) << endl;
//            }
//            if (i == 1) {
//                bytes[i] = (unary << (b + 1)) | r;
//                bytes[i] = bytes[i] << (8 - (((q % 8) + b + 1)));
//                cout << "4: " << bitset<8>(bytes[i]) << endl;
//            }
//            //cout << "byte: " << bitset<8>(bytes[i]) << endl;
//            bsw.writeNBits(bytes[i], 8);
//            i--;
//        }
//    } else {
//        uint64_t i=nBytes_q_r;
//        while(i>0) {
//            //cout << "i: " << i << endl;
//
//            if(i>3) {
//                /* when the number of bytes needed to store both q and r > 2, we need to perform
//                 * 8^(i-2) shifts to obtain the 8 bits of any byte with index>2 */
//                bytes[i] = unary >> (uint64_t)pow(8,i-2) >> (q % 8);
//            }
//            if (i == 3) {
//                bytes[i] = unary >> (q % 8);
//            }
//            if (i == 2) {
//                bytes[i] = unary << (8-(q%8));
//                uint8_t free = (8-(q%8))-1;
//                if(free!=0) {
//                    bytes[i] = bytes[i] | (r>>b-free);
//                    bytes[i-1] = (r & (b-free << 1)) << 8-(b-free);
//                } else {
//                    bytes[i-1] = r << (8-b);
//                }
//            }
//            cout << "byte: " << bitset<8>(bytes[i]) << endl;
//            bsw.writeNBits(bytes[i], 8);
//            i--;
//        }
//    }

    bsw.close();
}

