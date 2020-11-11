#include "BitStream_Write.h"
#include "BitStream_Read.h"
#include "GolombEncoder.h"
#include "GolombDecoder.h"
#include <bitset>

using namespace std;

int main() {

    uint32_t m = 4;
    GolombEncoder enc(m);
    enc.encode(206);
    GolombDecoder dec(m);
    uint64_t res = dec.decode();



    return 0;
}