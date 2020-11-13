/**
 * \brief A class that implements an entropy encoder using Golomb codes.
 *        Includes methods to encode both positive and negative numbers.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */

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
        * \param[in] m      Parameter m of the Golomb code
        */
        GolombEncoder(unsigned int m);

        /**
         * \brief Destructor.
         */
        virtual ~GolombEncoder();

        /**
        * \brief A function to encode both signed and unsigned numbers using the Golomb code.
        *        The representation of both signed and unsigned numbers is made by adding an
        *       additional bit before each quotient representation. Thus a total number
        *       of 1 + q + b bits is written in the bitstream to represent each coded value.
        * \param[in] value       The value to be encoded
        */
        void encode(signed int value);
    private:
        BitStream_Write bsw {"file.test"}; // file to write the coded bit sequence
        unsigned int m;                    // m parameter of the Golomb code
        unsigned int b;                    // number of bits needed to represent the remainder
};

#endif //DELIVERABLE_2_GOLOMB_ENCODE_H
