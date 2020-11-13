#include <CppUTest/TestHarness.h>
#include "GolombDecoder.h"
#include "GolombEncoder.h"

TEST_GROUP(Golomb)
{
    void setup()
    {

    }

    void teardown()
    {

    }
};

// Tests a normal integer input and checks if the values are expected

TEST(Golomb, Coding_Decoding_normal) {

    unsigned int m;
    signed int value, result;
    m = 4;
    value = 11;

    GolombEncoder encoder(m);
    encoder.encode(value);
    GolombDecoder decoder(m);
    result = decoder.decode();
    CHECK_EQUAL(value,result);

}

// Tests a negative integer input and checks if the values are the expected ones
TEST(Golomb, Coding_Decoding_negative) {

    unsigned int m;
    signed int value, result;
    m = 4;
    value = -11;

    GolombEncoder encoder(m);
    encoder.encode(value);
    GolombDecoder decoder(m);
    result = decoder.decode();
    CHECK_EQUAL(value,result);

}
// Tests when the input value is zero
TEST(Golomb, Coding_Decoding_zero) {

    unsigned int m;
    signed int value, result;
    m = 4;
    value = 0;

    GolombEncoder encoder(m);
    encoder.encode(value);
    GolombDecoder decoder(m);
    result = decoder.decode();
    CHECK_EQUAL(value,result);

}

