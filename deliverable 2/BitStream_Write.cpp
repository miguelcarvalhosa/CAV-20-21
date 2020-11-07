#include "BitStream_Write.h"


BitStream_Write::BitStream_Write(std::string fileName) {
    fp.open(fileName, std::fstream::out | std::fstream::binary);        // Open the file
    if(!fp) {
        std::cerr << "ERROR in BitStream_Write: Could not open file \"" << fileName << "\"." << std::endl;
    }
    buffer = 0x0;       // Initialize the buffer empty
    pos = 7;            // Point to the MSB of the buffer as the first free position
}


BitStream_Write::~BitStream_Write() {
    close();
}


void BitStream_Write::close() {
    fp.close();     // Close the file
}


/*
 * This function is implemented with low level bitwise operations using masks and shifts for performance optimization.
 */
void BitStream_Write::writeBit(uint8_t val) {
    if(val == 0 || val == 1) {      // Validate the argument 'val'
        if(pos >= 0 && pos <= 7) {  // If the buffer is not full, write the bit to the next available position
            buffer &= ~(1<<pos);    // Reset bit in position 'pos' on buffer
            buffer |= (val<<pos);   // Replace bit in position 'pos' on buffer with 'val'
            pos--;                  // Decrement the position pointer
        }
        if(pos == -1) {         // If the buffer is full, write the entire buffer to the file
            fp.write(reinterpret_cast<char *>(&buffer), sizeof(buffer));
            pos = 7;                // Reset the position pointer
            buffer = 0;             // Reset the buffer
        }
    }
    else {
        std::cerr << "ERROR in BitStream_Write::writeBit: invalid value of argument 'val'." << std::endl;
    }
}


void BitStream_Write::writeNBits(uint64_t val, uint8_t nbits) {
    if(nbits >= 1 && nbits <= 64) {                 // Validate the argument 'nbits'
        for (int i=nbits; i>=1; i--) {              // Iterate through each of the bits in the word 'val', from
                                                    // 'nbits' to the LSB
            uint8_t bit = (val >> (i-1)) & 0x1;     // Isolate each bit, using right shifts and a mask
            writeBit(bit);                          // Write each bit to the file, using writeBit()
        }
    }
    else {
        std::cerr << "ERROR in BitStream_Write::writeNBits: invalid value of argument 'nbits'." << std::endl;
    }
}
