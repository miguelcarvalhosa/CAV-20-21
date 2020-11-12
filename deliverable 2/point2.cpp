#include "BitStream_Write.h"
#include "GolombEncoder.h"
#include "GolombDecoder.h"

using namespace std;

int main() {

    unsigned int m;
    signed int value, result;
    m = 5;
    value = -1007;

    GolombEncoder encoder(m);
    encoder.encode(value);
    GolombDecoder decoder(m);

    result = decoder.decode();
    cout << "Valor a codificar  : " << value << endl;
    cout << "Valor descodificado: " << result << endl;

    return 0;
}