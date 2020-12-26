/**
 * \brief A class that implements an entropy decoder using Golomb codes.
 *        Includes methods to decode both positive and negative numbers.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */
#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include "BitStream_Read.h"

#ifndef DELIVERABLE_2_GOLOMBDECODER_H
#define DELIVERABLE_2_GOLOMBDECODER_H


class GolombDecoder {
    public:
        /**
        * \brief Constructor.
        *
        * \param[in] m      parameter of the Golomb code
        */
        GolombDecoder(unsigned int m, std::string fileName);

        /**
         * \brief Destructor.
         */
        virtual ~GolombDecoder();

        /**
        * \brief A function to update the m parameter used by the decoder
        * \param[in] m       new value for the m parameter
        */
        void update(unsigned int m);

        /**
        * \brief A function to decode both signed and unsigned numbers using the Golomb code.
        *        The representation of both signed and unsigned numbers is made by adding an
        *        additional bit before each quotient representation. Thus, all operations are
         *       performed in modulus and the sign is restored at the end.
        * \return     The signed decoded value
        */
        signed int decode();
        std::string decodeHeader(unsigned int nChars);

        /**
         * \brief A function that the allows the closing of the file that is being read. It should come
         *        after the whole file is read.
         */
        void close();
    private:
        std::string fileName;             // file used to read the coded bit sequence
        BitStream_Read bsr;
        unsigned int m;                   // m parameter of the Golomb code
        unsigned int b;                   // number of bits needed to represent the remainder
};


#endif //DELIVERABLE_2_GOLOMBDECODER_H
