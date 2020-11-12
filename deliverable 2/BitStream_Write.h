/**
 * \brief A class to write a stream of bits to a file.
 *        Includes methods to write single bits and strings.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */


#ifndef BITSTREAM_WRITE_H
#define BITSTREAM_WRITE_H

#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <string>


class BitStream_Write {

public:
    /**
     * \brief Constructor.
     *
     * \param[in] fileName      A string with the file name to be written
     */
    BitStream_Write(std::string fileName);

    /**
     * \brief Destructor.
     */
    virtual ~BitStream_Write();

    /**
     * \brief A function to close the bitstream.
     */
    void close();

    /**
     * \brief A function to write a bit to the file.
     *
     * \param[in] val       The value to be written
     */
    void writeBit(uint8_t val);

    /**
     * \brief A function to write a number of bits to the file.
     *        Can be used to write integer numbers with up to 64 bits.
     *
     * \param[in] val       The value to be written
     * \param[in] nbits     The number of bits to be written, in range [1 : 64]
     */
    void writeNBits(uint64_t val, uint8_t nbits);

    /**
     * \brief A function to write a string to the file in binary
     *
     * \param[in] str       The input string to be written
     */
     void writeString(const std::string &str);


private:
    uint8_t buffer;         // 8 bit buffer to store each wrote byte
    int8_t pos;             // Next free position on the buffer to write
    std::fstream fp;        // Pointer to the file
};


#endif //BITSTREAM_WRITE_H
