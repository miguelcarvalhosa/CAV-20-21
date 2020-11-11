//
// Created by joao on 09/11/20.
//
#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <iostream>

#include <bitset>
#include "BitStream_Read.h"

using namespace std;

#ifndef DELIVERABLE_2_GOLOMBDECODER_H
#define DELIVERABLE_2_GOLOMBDECODER_H


class GolombDecoder {
    public:
        /**
        * \brief Constructor.
        *
        * \param[in] fileName      A string with the file name of the input stream to be encoded
        */
        GolombDecoder(uint32_t m);

        /**
         * \brief Destructor.
         */
        virtual ~GolombDecoder();

        /**
        * \brief A function to
        *
        * \param[in] val       The value to be encoded
        */
        uint64_t decode();

    private:
        BitStream_Read bsr {"file.test"};
        uint32_t m;
    uint32_t b;
};


#endif //DELIVERABLE_2_GOLOMBDECODER_H
