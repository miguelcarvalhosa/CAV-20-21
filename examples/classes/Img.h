//
// Created by Antonio J. R. Neves on 16/10/2020.
//

#ifndef CLASSES_IMG_H
#define CLASSES_IMG_H

#include <iostream>

class Img {
private:
    unsigned int nRows, nCols;
    unsigned char *data;

public:
    Img(unsigned int _nRows, unsigned int _nCols) : nRows(_nRows), nCols(_nCols) {
        data = new unsigned char[nRows * nCols];
        // nRows = _nRows;
        // nCols = _nCols;
    }

    Img() {
        nRows = 5;
        nCols = 5;
        data = new unsigned char[nRows * nCols];
    }

    virtual ~Img() {
        delete []data;
    }

    unsigned int getNRows() const {
        return nRows;
    }

    unsigned int getNCols() const {
        return nCols;
    }

    unsigned char *getData() const {
        return data;
    }

    void print() const {
        for(int i = 0 ; i < nCols * nRows ; i++){
            std::cout << static_cast<int>(data[i]) << " ";
            if((i + 1) % nCols == 0)
                std::cout << std::endl;
        }


        std::cout << std::endl;
    }

    void putPixel(const unsigned int r, const unsigned int c, unsigned char value){
        if(r < 0 || c < 0 || c >= nCols || r >= nRows)
            throw std::out_of_range("out of range putPixel");

        data[r * nCols + c] = value;
    }
};


#endif //CLASSES_IMG_H
