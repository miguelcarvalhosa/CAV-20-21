#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <iostream>

#include "BitStream_Read.h"

using namespace std;

#ifndef DELIVERABLE_2_GOLOMBDECODER_H
#define DELIVERABLE_2_GOLOMBDECODER_H


class GolombDecoder {
    public:
        /**
        * \brief Constructor.
        *
        * \param[in] m      parameter of the encoder
        */
        GolombDecoder(unsigned int m);

        /**
         * \brief Destructor.
         */
        virtual ~GolombDecoder();

        /**
        * \brief A function to
        *
        * \param[in] val       The value to be decoded
        */
        signed int decode();

    private:
        BitStream_Read bsr {"file.test"};
        unsigned int m;
        unsigned int b;
};


#endif //DELIVERABLE_2_GOLOMBDECODER_H
