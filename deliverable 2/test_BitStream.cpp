#include <CppUTest/TestHarness.h>
#include "BitStream_Write.h"
#include "BitStream_Read.h"
#include <math.h>
#include <string>


TEST_GROUP(Bit_Stream)
{
    void setup()

    {

    }

    void teardown()
    {

    }
};

//Test Write and read of a single bit (multiple times)

TEST(Bit_Stream,Write_Read_bit)
{
    BitStream_Write bsw("Teste.test");
    for(int i=0;i<8;i++) {
        bsw.writeBit(1);

    }
    bsw.close();

    BitStream_Read bsr("Teste.test");
    uint8_t bit;
    for(int i=0;i<8;i++) {
        bit = bsr.readBit();
        CHECK_EQUAL(1,bit);
    }
    bsr.close();

}

// Test writing and reading n bits, being the input value an integer
TEST(Bit_Stream,Write_Read_nbit_uint)
{
    BitStream_Write bsw("Teste.test");
    uint64_t Test_val;
    uint8_t nbits;
    Test_val = 100;
    nbits = 9;
    bsw.writeNBits(Test_val,nbits);
    bsw.close();
    BitStream_Read bsr("Teste.test");
    uint64_t Result = bsr.readNBits(9);
    CHECK_EQUAL(100,Result);

}

// Test writing and reading n bits, being the input value an hexadecimal value
TEST(Bit_Stream,Write_Read_nbit_Hex)
{
    BitStream_Write bsw("Teste.test");
    uint64_t Test_val;
    uint8_t nbits;
    Test_val = 0xFFAE;
    nbits = 16;
    bsw.writeNBits(Test_val,nbits);
    bsw.close();
    BitStream_Read bsr("Teste.test");
    uint64_t Result = bsr.readNBits(16);

    CHECK_EQUAL(0XFFAE,Result);

}

// Tests the highest value possible according to the number of bits used
TEST(Bit_Stream,LargestValueFoNbits)
{
    BitStream_Write bsw("Teste.test");

    for(int nbits = 1; nbits < 10; nbits++) {
        bsw.writeNBits(pow(2,nbits)-1,nbits);
    }
    bsw.close();

    BitStream_Read bsr("Teste.test");
    for(int nbits = 1; nbits < 10; nbits++){

        uint64_t Result = bsr.readNBits(nbits);
        uint64_t Expected = pow(2,nbits) - 1 ;
        CHECK_EQUAL(Expected,Result);
    }

    bsr.close();
}

// Test string reading and writing
TEST(Bit_Stream,Write_Read_String)
{
    BitStream_Write bsw("Teste.test");
    std::string str = "String Correctly Written";
    bsw.writeString(str);
    bsw.close();

    BitStream_Read bsr("Teste.test");
    std::string result = "";
    bsr.readString(result, 24);
    CHECK_EQUAL_TEXT(str,result,"String Correctly Written");
    bsr.close();
}



/* Possivel adicionar se se puser erro quando esta situação acontece)
TEST(Bit_Stream,Write_Read_nbit) {
    BitStream_Write bsw("Teste.test");
    uint64_t val;
    uint8_t nbits;
    val = 10;
    nbits = 65;
    CHECK_THROWS(std::cerr, bsw.writeNBits(val, nbits));
    bsw.close();
}

*/













