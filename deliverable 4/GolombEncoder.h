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
        * \param[in] m         Parameter m of the Golomb code
        * \param[in] fileName  A string with the file name to be written by the encoder
        */
        GolombEncoder(unsigned int m, std::string fileName);

        /**
         * \brief Destructor.
         */
        virtual ~GolombEncoder();

        /**
        * \brief A function to update the m parameter used by the encoder
        * \param[in] m       new value for the m parameter
        */
        void update(unsigned int m);

        /**
        * \brief A function to encode both signed and unsigned numbers using the Golomb code.
        *        The representation of both signed and unsigned numbers is made by adding an
        *       additional bit before each quotient representation. Thus a total number
        *       of 1 + q + b bits is written in the bitstream to represent each coded value.
        * \param[in] value       The value to be encoded
        */
        void encode(signed int value);

        void close();
    private:
        std::string fileName;               // file to write the coded bit sequence
        BitStream_Write bsw;
        unsigned int m;                    // m parameter of the Golomb code
        unsigned int b;                    // number of bits needed to represent the remainder
};

#endif //DELIVERABLE_2_GOLOMB_ENCODE_H
