#include "BitStream_Read.h"

BitStream_Read::BitStream_Read() {

}

BitStream_Read::BitStream_Read(std::string fileName) {
    fp.open(fileName, std::fstream::in | std::fstream::binary);     // Open the file
    if(!fp) {
        std::cerr << "ERROR in BitStream_Read: Could not open file \"" << fileName << "\"." << std::endl;
    }
    buffer = 0x0;       // Initialize the buffer empty
    pos = -1;           // Initialize the position pointer
}


BitStream_Read::~BitStream_Read() {
    close();
}


void BitStream_Read::close() {
    fp.close();     // Close the file
}

void BitStream_Read::setFileName(std::string fileName) {
    fp.open(fileName, std::fstream::in | std::fstream::binary);     // Open the file
    if(!fp) {
        std::cerr << "ERROR in BitStream_Read: Could not open file \"" << fileName << "\"." << std::endl;
    }
    buffer = 0x0;       // Initialize the buffer empty
    pos = -1;           // Initialize the position pointer
}

uint8_t BitStream_Read::readBit() {
    uint8_t bit = 0xFF;         // Initialize the return variable with an invalid value
    if(pos == -1) {             // If the buffer is empty, read a byte from the file
        fp.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));     // Read a byte from the file to the buffer
        pos = 7;        // Reset the position pointer
    }
    if(pos >= 0 && pos <= 7) {      // If the buffer is not empty, read the bit from the next available position
        bit = (buffer >> pos) & 0x1;    // Isolate each bit, using shifts and a mask
        pos--;                          // Decrement the position pointer
    }
    if(bit == 0xFF) {       // If the return value is invalid, print an error message
        std::cerr << "ERROR in BitStream_Read::readBit: invalid return value." << std::endl;
    }
    return bit;
}


uint64_t BitStream_Read::readNBits(uint8_t nbits) {
    uint64_t word = 0;                      // Initialize the return variable
    if(nbits >= 1 && nbits <= 64) {         // Validate the argument 'nbits'
        for(int i=nbits-1; i>=0; i--) {     // Iterate through each bit position of the word
            uint8_t bit = readBit();        // Read the bit from the file
            word &= ~(1<<i);                // Reset bit in position 'i' on 'word'
            word |= (bit<<i);               // Replace bit in position 'i' on 'word' with 'bit'
        }
    }
    else {
        std::cerr << "ERROR in BitStream_Read::readNBits: invalid value of argument 'nbits'." << std::endl;
    }
    return word;
}


void BitStream_Read::readString(std::string &str, uint8_t nchars) {
    std::string _str;                               // Create a string
    for(int i=0; i<nchars; i++) {                   // Iterate through each 'nchars'
        uint8_t readChar = readNBits(8);      // Read a character (8 bits) from the file
        _str.append(1, readChar);               // Append the character to the string
    }
    str = _str;                                     // Copy the created string to the output string
}

