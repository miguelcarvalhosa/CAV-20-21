#include "BitStream_Write.h"
#include "BitStream_Read.h"
#include <bitset>

using namespace std;

int main() {

    string str("Hello World");
    BitStream_Write bsw("file.test");
    bsw.writeString(str);
    bsw.close();

    /*
    BitStream_Write bsw("file.test");
    bsw.writeNBits(65535, 16);
    bsw.writeNBits(0xABCD, 16);
    bsw.writeNBits(8, 8);
    bsw.close();

    BitStream_Read bsr("file.test");
    //for(int i=0; i<40; i++) {
    //    cout << (char)(48+bsr.readBit());
    //}
    uint64_t word = bsr.readNBits(40);
    cout << bitset<40>(word);
    bsr.close();
    */
    return 0;
}
