//
// Created by joao on 09/11/20.
//

#include "GolombDecoder.h"
#include "math.h"
#include <bitset>


GolombDecoder::GolombDecoder(uint32_t m) {
    this->m = m;
    b = ceil(log2(m));
    cout << "b: " << b << endl;
};

GolombDecoder::~GolombDecoder() {
    // ?
}
uint64_t GolombDecoder::decode() {

    uint32_t q, r;
    uint64_t word = bsr.readNBits(64);
    cout << bitset<64>(word) << endl;
    bsr.close();
}

