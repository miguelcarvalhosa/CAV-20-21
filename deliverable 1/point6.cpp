/**
 * \file point6.cpp
 *
 * \brief Contains the resolution of points 6 and 8 of Deliverable 1.
 *        This program reads an audio file in .wav format, applies a quantization algorithm to reduce the number of bits
 *        used to represent each audio sample and saves the quantized file.
 *        To use the program, the user must pass three arguments: the input file path, the output file path and the
 *        number of bits to use in the quantizer.
 *
 *        Usage: point6 <input file> <output file> <nbits>
 *
 *        At the end, the program prints the signal-to-noise ration (SNR), as well as the maximum per sample absolute
 *        error.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */


#include <iostream>
#include <vector>
#include <math.h>
#include <sndfile.hh>


using namespace std;


/*
 * Constants
 */
constexpr size_t FRAMES_BUFFER_SIZE = 65536;    // Buffer for reading/writing frames


/*
 * Function Prototypes
 */
void printFileInfo(SndfileHandle sndfilein);
short quantizer(unsigned short nbits, long min, long max, short value);


int main(int argc, char *argv[]) {

    // Validate the number of arguments
    if(argc < 4) {
        cerr << "Usage: point6 <input file> <output file> <nbits>" << endl;
        return 1;
    }

    // Validate the input file
    SndfileHandle sndFileIn { argv[argc-3] };
    if(sndFileIn.error()) {
        cerr << "Error: invalid input file" << endl;
        return 1;
    }
    if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format" << endl;
        return 1;
    }
    if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format" << endl;
        return 1;
    }


    // Print some info about the input file
    cout << "Input File - ";
    printFileInfo(sndFileIn);

    cout << "Number of bits to use: " << stoi(argv[argc-1]) << endl;

    // Create the output file handler
    SndfileHandle sndFileOut { argv[argc-2], SFM_WRITE, sndFileIn.format(),
                               sndFileIn.channels(), sndFileIn.samplerate() };

    // Validate the output file
    if(sndFileOut.error()) {
        cerr << "Error: invalid output file" << endl;
        return 1;
    }


    size_t nFrames;     // Number of frames to read at each iteration
    long energyIn = 0;      // Input file energy value
    long energyNoise = 0;   // Noise energy value
    int maxError = 0;       // Maximum per sample absolute error
    int nchannels = sndFileIn.channels();       // Number of channels of the input file

    vector<short> samplesIn(FRAMES_BUFFER_SIZE * nchannels);        // Vector to hold the input file samples
    vector<short> samplesQuant(FRAMES_BUFFER_SIZE * nchannels);     // Vector to hold the quantized samples

    // Read the input file, FRAMES_BUFFER_SIZE samples at a time
    while((nFrames = sndFileIn.readf(samplesIn.data(), FRAMES_BUFFER_SIZE))) {
        for(size_t i = 0; i < nFrames*nchannels; i++) {             // Iterate through each read sample
            // Quantize each sample
            samplesQuant[i] = quantizer(stoi(argv[argc-1]), 0, pow(2, 16), samplesIn[i]);
            int noise = abs(samplesIn[i] - samplesQuant[i]);    // Calculate the noise value
            if(noise > maxError) {
                maxError = noise;
            }
            energyIn += pow(samplesIn[i], 2);       // Accumulate the signal energy
            energyNoise += pow(noise, 2);           // Accumulate the noise energy
        }
        sndFileOut.writef(samplesQuant.data(), nFrames);        // Write the quantized samples to the output file
    }


    // Print the results
    double SNR = 10*log10((double)(energyIn) / (double)(energyNoise));
    cout << "SNR: " << SNR << " dB" << endl;
    cout << "Max per sample absolute error: " << maxError << endl;


    return 0;
}


/**
 * \brief A function to print some information  about the audio file.
 *
 * \param file          Handler for the audio file
 *
 */
void printFileInfo(SndfileHandle file) {
    cout << "File has:" << endl;
    cout << '\t' << file.frames() << " frames" << endl;
    cout << '\t' << file.samplerate() << " samples per second" << endl;
    cout << '\t' << file.channels() << " channels" << endl;
}


/**
 * \brief A function to quantize a sample.
 *
 * \param nbits         Number of bits to represent the quantized sample
 * \param min           Mininum value of the input sample range
 * \param max           Maximun value of the input sample range
 * \param value         Sample to be quantized
 *
 * \return The quantized value
 *
 */
short quantizer(unsigned short nbits, long min, long max, short value) {
    short delta = abs(max-min) / pow(2, nbits);     // Delta, the quantizer level interval, is calculated as
    // 'delta = A / 2^b', where A is the input range and b is the
    // number of bits
    return min + floor((value-min)/delta)*delta + delta/2;
}