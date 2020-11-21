#include "BitStream_Write.h"
#include "GolombEncoder.h"
#include "GolombDecoder.h"

using namespace std;

int main() {

    unsigned int m;
    signed int value, result;
    m = 876;
    value = -2467;

    GolombEncoder encoder(m, "file.test");
    encoder.encode(value);
    GolombDecoder decoder(m, "file.test");

    result = decoder.decode();
    cout << "Valor a codificar  : " << value << endl;
    cout << "Valor descodificado: " << result << endl;

    return 0;
}