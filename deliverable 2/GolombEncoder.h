//
// Created by joao on 09/11/20.
//
#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include "BitStream_Write.h"

#ifndef DELIVERABLE_2_GOLOMB_ENCODE_H
#define DELIVERABLE_2_GOLOMB_ENCODE_H


class GolombEncoder {
    public:
        /**
        * \brief Constructor.
        *
        * \param[in] fileName      A string with the file name of the input stream to be encoded
        */
        GolombEncoder(uint32_t m);

        /**
         * \brief Destructor.
         */
        virtual ~GolombEncoder();

        /**
        * \brief A function to
        *
        * \param[in] val       The value to be encoded
        */
        void encode(uint64_t val);
    private:
        BitStream_Write bsw {"file.test"};
        uint32_t m;
        uint32_t b;
};


#endif //DELIVERABLE_2_GOLOMB_ENCODE_H