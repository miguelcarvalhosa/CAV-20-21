#include "BitStream_Write.h"
#include "BitStream_Read.h"
#include <bitset>

using namespace std;

int main() {

    BitStream_Write bsw("file.test");
    bsw.writeNBits(65535, 16);
    bsw.writeNBits(0xABCD, 16);
    bsw.writeNBits(8, 8);
    string str("Hello World");
    bsw.writeString(str);
    bsw.writeNBits(0b0011, 4);
    bsw.close();

    BitStream_Read bsr("file.test");
    uint64_t word = bsr.readNBits(40);
    cout << bitset<40>(word) << endl;
    string str1;
    bsr.readString(str1, str.length());
    cout << str1 << endl;
    bsr.close();

    return 0;
}
