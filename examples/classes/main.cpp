#include <iostream>

#include "Image.h"

int main() {
    Image i(3, 3);

    try {
        i.putPixel(300, 50);
    }
    catch (std::out_of_range e){
        std::cout << e.what() << std::endl;
    }

    i.printData();

    return 0;
}