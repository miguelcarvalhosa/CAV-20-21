/**
 * \brief A class to read a stream of bits from a file.
 *        Includes methods to read single bits and strings.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */


#ifndef BITSTREAM_READ_H
#define BITSTREAM_READ_H

#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <iostream>


class BitStream_Read {

public:

    /**
     * \brief Constructor.
     *
     * \param[in] fileName      A string with the file name to be read
     */
    BitStream_Read(std::string fileName);

    /**
     * \brief Destructor.
     */
    virtual ~BitStream_Read();

    /**
     * \brief A function to close the bitstream.
     */
    void close();

    /**
     * \brief A function to read a bit from the file.
     *
     * \return The value read
     *  \retval 0
     *  \retval 1
     *  \retval 0xFF    Invalid return value
     */
    uint8_t readBit();

    /**
     * \brief A function to read a number of bits from the file.
     *
     * \param[in] nbits     The number of bits to be written
     * \return The value read
     */
    uint64_t readNBits(uint8_t nbits);


private:
    uint8_t buffer;         // 8 bit buffer to store each read byte
    int8_t pos;             // Next free position on the buffer to read
    std::fstream fp;        // Pointer to the file
};


#endif //BITSTREAM_READ_H
