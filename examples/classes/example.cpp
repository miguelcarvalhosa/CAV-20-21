//
// Created by Antonio J. R. Neves on 16/10/2020.
//
#include "Img.h"
#include <iostream>

using namespace std;

int main() {

    Img i1;
    Img i2(1024, 768);

    cout << i1.getNCols() << endl;

    try {
        i1.putPixel(10, 1, 255);
    }
    catch (std::out_of_range e){
        std::cout << e.what() << std::endl;
    }

    i1.print();

    string s = string("CAV");
    cout << "ola" << s << std::endl;

    return 0;
}